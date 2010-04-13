<?php
    
class YfrogUploader
{
    /**
     * API endpoint
     */
    const YFROG_API_URL = 'http://yfrog.com/api';

    /**
     * Default connection and response timeout
     */
    const YFROG_API_TIMEOUT = 10;

    /**
     * Error codes,
     * 3XXX are client errors
     * 2XXX are ImageShack-specified errors
     * 1XXX are standard errors
     */
    const YFROG_ERROR_BAD_CREDENTIALS =       1001;
    const YFROG_ERROR_NO_FILE =               1002;
    const YFROG_ERROR_SIZE_TOO_BIG =          1004;

    const YFROG_ERROR_WRONG_ACTION =          2001;
    const YFROG_ERROR_UPLOAD_FAILED =         2002;
    const YFROG_ERROR_STATUS_UPDATE_FAILED =  2003;

    const YFROG_ERROR_IO_ERROR =              3001;
    const YFROG_ERROR_MALFORMED_XML =         3002;



    /**
     * Uploads file to yfrog.com. 
     * Requirements: simplexml, curl
     * @param filename filename to upload
     * @param username Twitter username
     * @param password Twitter password
     * @param tags comma-separated list of tags
     * @param public is uploaded media should be public or not
     * @param timeout connection and response timeout
     * @param key developer key, see http://code.google.com/p/imageshackapi/wiki/DeveloperKey
     * @return array 
     *  stat: true/false; true indicates success, false - error
     *  code: error code  (only if stat = false)
     *  msg: error message (only if stat = false)
     *  mediaid media identifier (only if stat = true)
     *  mediaurl media URL (only if stat = true)
     */
    function upload($filename,
                    $username,
                    $password,
                    $tags = '',
                    $public = true,
                    $timeout = self::YFROG_API_TIMEOUT,
                    $key = null)
    {
        $request = array
        (
            'username' => $username,
            'password' => $password,
            'tags'     => $tags,
            'public'   => $public ? 'yes' : 'no',
            'media'    => '@' . $filename
        );
        if ($key)
            $request['key'] = $key;

        return $this->exec('upload', $request, $timeout);
    }

    /**
     * Transloads file to yfrog.com. 
     * Requirements: simplexml, curl
     * @param url URL to transload
     * @param username Twitter username
     * @param password Twitter password
     * @param tags comma-separated list of tags
     * @param public is uploaded media should be public or not
     * @param timeout connection and response timeout
     * @param key developer key, see http://code.google.com/p/imageshackapi/wiki/DeveloperKey
     * @return array 
     *  stat: true/false; true indicates success, false - error
     *  code: error code  (only if stat = false)
     *  msg: error message (only if stat = false)
     *  mediaid media identifier (only if stat = true)
     *  mediaurl media URL (only if stat = true)
     */
    function transload($url,
                       $username,
                       $password,
                       $tags = '',
                       $public = true,
                       $timeout = self::YFROG_API_TIMEOUT,
                       $key = null)
    {
        $request = array
        (
            'username' => $username,
            'password' => $password,
            'tags'     => $tags,
            'public'   => $public ? 'yes' : 'no',
            'url'      => $url
        );
        if ($key)
            $request['key'] = $key;

        return $this->exec('upload', $request, $timeout);
    }


    /**
     * Uploads file to yfrog.com and posts message to Twitter. 
     * Requirements: simplexml, curl
     * @param filename filename to upload
     * @param message message to send
     * @param username Twitter username
     * @param password Twitter password
     * @param tags comma-separated list of tags
     * @param public is uploaded media should be public or not
     * @param timeout connection and response timeout
     * @param source optional Twitter 'source' parameter to indicate tweets as 'Posted from SOURCE'.
     * If not specified, 'yfrog' is used
     * @param key developer key, see http://code.google.com/p/imageshackapi/wiki/DeveloperKey
     * @return array 
     *  stat: true/false; true indicates success, false - error
     *  code: error code  (only if stat = false)
     *  msg: error message (only if stat = false)
     *  mediaid media identifier (only if stat = true)
     *  mediaurl media URL (only if stat = true)
     */
    function uploadAndPost($filename,
                           $message,
                           $username,
                           $password,
                           $tags = '',
                           $public = true,
                           $timeout = self::YFROG_API_TIMEOUT,
                           $source = 'yfrog',
                           $key = null)
    {
        $request = array
        (
            'username' => $username,
            'password' => $password,
            'tags'     => $tags,
            'public'   => $public ? 'yes' : 'no',
            'media'    => '@' . $filename,
            'message'  => $message,
            'source'   => $source
        );
        if ($key)
            $request['key'] = $key;

        return $this->exec('uploadAndPost', $request, $timeout);
    }


    /**
     * Transloads file to yfrog.com and posts message to Twitter. 
     * Requirements: simplexml, curl
     * @param url URL to upload
     * @param message message to send
     * @param username Twitter username
     * @param password Twitter password
     * @param tags comma-separated list of tags
     * @param public is uploaded media should be public or not
     * @param timeout connection and response timeout
     * @param source optional Twitter 'source' parameter to indicate tweets as 'Posted from SOURCE'.
     * If not specified, 'yfrog' is used
     * @param key developer key, see http://code.google.com/p/imageshackapi/wiki/DeveloperKey
     * @return array 
     *  stat: true/false; true indicates success, false - error
     *  code: error code  (only if stat = false)
     *  msg: error message (only if stat = false)
     *  mediaid media identifier (only if stat = true)
     *  mediaurl media URL (only if stat = true)
     */
    function transloadAndPost($url,
                              $message,
                              $username,
                              $password,
                              $tags = '',
                              $public = true,
                              $timeout = self::YFROG_API_TIMEOUT,
                              $source = 'yfrog',
                              $key = null)
    {
        $request = array
        (
            'username' => $username,
            'password' => $password,
            'tags'     => $tags,
            'public'   => $public ? 'yes' : 'no',
            'url'      => $url,
            'message'  => $message,
            'source'   => $source
        );
        if ($key)
            $request['key'] = $key;

        return $this->exec('uploadAndPost', $request, $timeout);
    }



    private function exec($action,
                          $request,
                          $timeout)
    {
        $handle = @curl_init(self::YFROG_API_URL . '/'. $action);
        if (!$handle)
            return array('stat' => false, 'code' => YFROG_ERROR_IO_ERROR, 'msg' => 'Unable to initialize CURL');

        curl_setopt($handle, CURLOPT_HEADER, 0);
        curl_setopt($handle, CURLOPT_CONNECTTIMEOUT, $timeout);
        curl_setopt($handle, CURLOPT_TIMEOUT, $timeout);
        curl_setopt($handle, CURLOPT_RETURNTRANSFER, 1);
        curl_setopt($handle, CURLOPT_FOLLOWLOCATION, 1);
        curl_setopt($handle, CURLOPT_POSTFIELDS, $request);

        $response = curl_exec($handle);

        $error = curl_errno($handle);
        if ($error)
        {
            $ret = array('stat' => false, 'code' => YFROG_ERROR_IO_ERROR, 'msg' => curl_error($handle) . ' [' . $error . ']');
            curl_close($handle);
            return $ret;
        }

        curl_close($handle);

        $xml = @simplexml_load_string($response);
        if (!$xml)
            return array('stat' => false, 'code' => YFROG_ERROR_MALFORMED_XML, 'msg' => 'Malformed XML is received as response');

        if (@$xml->attributes()->stat == 'fail')
        {
            return array('stat' => false, 'code' => $xml->err->attributes()->code, 'msg' => $xml->err->attributes()->msg);
        }
        elseif (@$xml->attributes()->stat == 'ok')
        {
            return array
            (
                'stat'      => true,
                'mediaid'   => $xml->mediaid,
                'mediaurl'  => $xml->mediaurl,
                'statusid'  => @$xml->statusid,
                'userid'    => @$xml->userid
            );
        }
        else
            return array('stat' => false, 'code' => YFROG_ERROR_MALFORMED_XML, 'msg' => 'Unexpected XML is received as response');

    }
    
}
?>