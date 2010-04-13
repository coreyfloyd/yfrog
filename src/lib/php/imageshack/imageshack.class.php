<?php
    
    require_once('multipost.class.php');

    class ImageShackUploader
    {
        /**
         * API endpoints
         */
        const IMAGESHACK_API_URL       = 'http://imageshack.us/upload_api.php';
        const IMAGESHACK_VIDEO_API_URL = 'http://render.imageshack.us/upload_api.php';

        /**
         * Default connection and response timeout
         */
        const IMAGESHACK_API_TIMEOUT = 10;

        protected $timeout = self::IMAGESHACK_API_TIMEOUT;

        protected $developer_key;
        protected $cookie;

        /**
         * Constructs new uploader using specified developer key and optional cookies
         * @param string developer_key developer key to use
         * @param cookie optional ImageShack cookies
         */
        function __construct($developer_key, $cookie = null, $timeout = self::IMAGESHACK_API_TIMEOUT)
        {
            $this->developer_key = $developer_key;
            $this->cookie        = $cookie;
            $this->timeout       = $timeout;
        }


        /**
         * Uploads specified file
         * @param string file file to upload
         * @param string optsize optional resize options, WIDTHxHEIGHT or 'resample'
         * @param boolean removeBar optional, if set to true, no information bar is available on generated thumbnail
         * @param string tags optional, comma-separated tags
         * @param boolean public optional, if specified then image visibility will be set to public or private
         * @param string contentType optional, file content type, if not specified we'll try to guess it by extension
         * @param string frameFilename optional video frame JPEG image
         */
        function upload($file, 
                        $optsize = null,
                        $removeBar = true,
                        $tags = null,
                        $public = null,
                        $contentType = null,
                        $frameFilename = null)
        {
            if (!$contentType)
                $contentType = $this->detectFileContentType($file);
            if (strpos($contentType, 'image/') === 0)
                $endpoint = self::IMAGESHACK_API_URL;
            else
                $endpoint = self::IMAGESHACK_VIDEO_API_URL;

            $params = array();
            $params[] = new filepart('fileupload', $file, basename($file), $contentType, 'iso-8859-1');
            if ($frameFilename)
                $params[] = new filepart('frmupload', $frameFilename, basename($frameFilename), 'image/jpeg', 'iso-8859-1');
            if ($optsize)
            {
                $params[] = new stringpart('optimage', 1);
                $params[] = new stringpart('optsize', $optsize);
            }
            if ($tags)
                $params[] = new stringpart('tags', $tags);
            $params[] = new stringpart('rembar', $removeBar ? 'yes' : 'no');
            if ($public !== null)
                $params[] = new stringpart('public', $public ? 'yes' : 'no');
            if ($this->cookie)
                $params[] = new stringpart('cookie', $this->cookie);
            $params[] = new stringpart('key', $this->developer_key);

            return $this->exec($endpoint, $params);
        }

        /**
         * Transloads URL
         * @param url URL to transload
         * @param string optsize optional resize options, WIDTHxHEIGHT or 'resample'
         * @param boolean removeBar optional, if set to true, no information bar is available on generated thumbnail
         * @param string tags optional, comma-separated tags
         * @param boolean public optional, if specified then image visibility will be set to public or private
         */
        function transload($url,
                           $optsize = null,
                           $removeBar = true,
                           $tags = null,
                           $public = null)
        {
            $contentType = $this->detectUrlContentType($url);
            if (!$contentType)
                return false;
            if (strpos($contentType, 'image/') === 0)
                $endpoint = self::IMAGESHACK_API_URL;
            else
                $endpoint = self::IMAGESHACK_VIDEO_API_URL;

            $params = array();
            $params[] = new stringpart('url', $url);
            if ($optsize)
            {
                $params[] = new stringpart('optimage', 1);
                $params[] = new stringpart('optsize', $optsize);
            }
            if ($tags)
                $params[] = new stringpart('tags', $tags);
            $params[] = new stringpart('rembar', $removeBar ? 'yes' : 'no');
            if ($public)
                $params[] = new stringpart('public', $public ? 'yes' : 'no');
            if ($this->cookie)
                $params[] = new stringpart('cookie', $this->cookie);
            $params[] = new stringpart('key', $this->developer_key);
            return $this->exec($endpoint, $params);
        }

        private function detectFileContentType($name)
        {
            $ext = strtolower(substr($name, strrpos($name, '.') + 1));
            switch ($ext)
            {
                case 'jpg':
                case 'jpeg':
                    return 'image/jpeg';
                case 'png':
                case 'bmp':
                case 'gif':
                    return 'image/' . $ext;
                case 'tif':
                case 'tiff':
                    return 'image/tiff';
                case 'mp4':
                    return 'video/mp4';
                case 'mov':
                    return 'video/quicktime';
                case '3gp':
                    return 'video/3gpp';
                case 'avi':
                    return 'video/avi';
                case 'wmv':
                    return 'video/x-ms-wmv';
                case 'mkv':
                    return 'video/x-matroska';

            }
            return 'application/octet-stream';
        }

        private function detectUrlContentType($url)
        {
            $ch = @curl_init($url);
            
            if ($ch) 
            {
                curl_setopt($ch, CURLOPT_HEADER, 1);
                curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 10);
                curl_setopt($ch, CURLOPT_TIMEOUT, 10);
                curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
                curl_setopt($ch, CURLOPT_NOBODY, 1);
                curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1);
                curl_setopt($ch, CURLOPT_USERAGENT, 'ImageShack Image Fetcher 1.1');
                
                $response = curl_exec($ch);

                $is_err = curl_errno($ch);
                if ($is_err)
                {             
                    @curl_close($ch);
                    return false;
                }
                $code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
                if ($code != 200)
                {
                    @curl_close($ch);
                    return false;
                }
                $ret = curl_getinfo($ch, CURLINFO_CONTENT_TYPE);
                @curl_close($ch);
                return $ret;
            }
            else
                return false;
        }

        private function exec($endpoint,
                              $request)
        {
            $response = multipost($endpoint, 
                                  $request, 
                                  $errno, 
                                  $errormessage,
                                  $this->timeout);

            $xml = @simplexml_load_string($response);
            if (!$xml)
            {
                //error_log('Failed to parse XML: ' . $response);
                return false;
            }

            if (!@$xml->error && !@$xml->files)
            {
                //error_log('Unexpected XML');
                return false;
            }

            return $xml;
        }
        

    }
?>