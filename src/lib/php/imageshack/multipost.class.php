<?php

define ('BOUNDARY', "----------------314159265358979323846"); 
//define ('BOUNDARY', "---------------------------7d63bf2ef300f2"); 

class part
{
  function part()
  {
  }

  function getBoundary()
  {
    return BOUNDARY;
  }

  function getCharSet()
  {
    return 'iso-8859-1';
  }

  function getContentType()
  {
    return 'application/octet-stream';
  }

  function getName()
  {
    return NULL;
  }

  function getTransferEncoding()
  {
    return NULL;
  }

  function getLength()
  {
    $ret  = $this->getStartLength();
    $ret += $this->getDispositionHeaderLength();
    $ret += $this->getContentTypeHeaderLength();
    $ret += $this->getTransferEncodingHeaderLength();
    $ret += $this->getEndOfHeaderLength();
    $ret += $this->getDataLength();
    $ret += $this->getEndLength();
    return $ret;
  }

  function send($h)
  {
    $this->sendStart($h);
    $this->sendDispositionHeader($h);
    $this->sendContentTypeHeader($h);
    $this->sendTransferEncodingHeader($h);
    $this->sendEndOfHeader($h);
    $this->sendData($h);
    $this->sendEnd($h);
  }

  function sendContentTypeHeader($h)
  {
    $contentType = $this->getContentType();
    if(!empty($contentType))
    {
        fwrite($h, "\r\n");
        fwrite($h, "Content-Type: ");
        fwrite($h, $contentType);
        $charset = $this->getCharSet();
        if (!empty($charset))
        {
          fwrite($h, "; charset=");
          fwrite($h, $charset);
        }
    }
  }

  function getContentTypeHeaderLength()
  {    
    $contentType = $this->getContentType();
    if(!empty($contentType))
    {
        $result  =  2;
        $result += strlen("Content-Type: ");
        $result += strlen($contentType);
        $charset = $this->getCharSet();
        if (!empty($charset))
        {
          $result += strlen("; charset=");
          $result += strlen($charset);
        }
        return $result;
    }
    else
      return 0;
  }


  function sendData($h)
  {
  }

  function getDataLength()
  {
    return 0;
  }

  function sendDispositionHeader($h)
  {
    fwrite($h, "Content-Disposition: form-data; name=");
    fwrite($h, "\"");
    fwrite($h, $this->getName());
    fwrite($h, "\"");
  }

  function getDispositionHeaderLength()
  {
    $result  = strlen("Content-Disposition: form-data; name=");
    $result += 1;
    $result += strlen($this->getName());
    $result += 1;
    return $result;
  }


  function sendEnd($h)
  {
    fwrite($h, "\r\n");
  }

  function getEndLength()
  {
    return 2;
  }


  function sendEndOfHeader($h)
  {
    fwrite($h, "\r\n\r\n");
  }

  function getEndOfHeaderLength()
  {
    return 4;
  }


  function sendStart($h)
  {
    fwrite($h, "--");
    fwrite($h, $this->getBoundary());
    fwrite($h, "\r\n");
  }

  function getStartLength()
  {
    $result  = 2;
    $result += strlen($this->getBoundary());
    $result += 2;
    return $result;
  }


  function sendTransferEncodingHeader($h)
  {
    $transferEncoding = $this->getTransferEncoding();
    if(!empty($transferEncoding))
    {
      fwrite($h, "\r\n");
      fwrite($h, "Content-Transfer-Encoding: ");
      fwrite($h, $transferEncoding);
    }
  }

  function getTransferEncodingHeaderLength()
  {
    $transferEncoding = $this->getTransferEncoding();
    if(!empty($transferEncoding))
    {
      $result  = 2;
      $result += strlen("Content-Transfer-Encoding: ");
      $result += strlen($transferEncoding);
      return $result;
    }
    else
      return 0;
  }

}

class stringpart extends part
{
  var $name;
  var $data;

  function stringpart($name, $data)  
  {
    $this->name = $name;
    $this->data = $data;
  }

  function getContentType()
  {
    return NULL;
  }

  function getName()
  {
    return $this->name;
  }

  function getTransferEncoding()
  {
    return "8bit";
    //return NULL;
  }

  function sendData($h)
  {
    fwrite($h, $this->data);
  }

  function getDataLength()
  {
    return strlen($this->data);
  }

}

class filepart extends part
{
  var $name;
  var $dataFileName;
  var $contentType;
  var $charset;

  function filepart($name, $dataFileName, $fileName, $contentType, $charset)
  {
    $this->name = $name;
    $this->dataFileName = $dataFileName;
    $this->fileName = $fileName;
    $this->contentType = $contentType;
    $this->charset = $charset;
  }

  function getCharSet()
  {
    return $this->charset;
  }

  function getContentType()
  {
    return $this->contentType;
  }

  function getName()
  {
    return $this->name;
  }

  function getTransferEncoding()
  {
    //return "binary";
    return NULL;
  }

  function sendData($h)
  {
    // TODO
    $fh = @fopen($this->dataFileName, 'rb');
    if ($fh)
    {
        stream_copy_to_stream($fh, $h);
        fclose($fh);
    }
  }

  function getDataLength()
  {
    return @filesize($this->dataFileName);
  }


  function sendDispositionHeader($h)
  {
    parent::sendDispositionHeader($h);
    if (!empty($this->fileName))
    {
      fwrite($h, "; filename=\"");
      fwrite($h, $this->fileName);
      fwrite($h, "\"");
    }
  }

  function getDispositionHeaderLength()
  {
    $ret = parent::getDispositionHeaderLength();
    if (!empty($this->fileName))
    {
      $ret += strlen("; filename=\"");
      $ret += strlen($this->fileName);
      $ret += 1;
    }
    return $ret;
  }


}

// posts specified data (array of name->value)
// to specified server

function multipost($url, 
                   $data, 
                   &$errno, 
                   &$errormessage,
                   $timeout = 2,
                   $headers = null)
{
  // Build post data
  $postdata = "--" . part::getBoundary() . "--\r\n";
  $postdatalen = strlen($postdata);

  for ($i = 0; $i < count($data); $i++)
    $postdatalen += $data[$i]->getLength();

  // identify server and location from URL
  $urlinfo = parse_url($url);

  $header  = "POST " . $urlinfo['path'] . " HTTP/1.0\r\n";
  $header .= "Host: " . $urlinfo['host'] . "\r\n";

  if ($headers != null)
  { 
    foreach ($headers as $name=>$value)
        $header .= $name . ':' . $value . "\r\n";
  }
  $header .= "Content-type: multipart/form-data; boundary=" . part::getBoundary() . "\r\n";
  $header .= "Content-length: " .  $postdatalen . "\r\n\r\n";

  if (isset($urlinfo['port']))
    $port = $urlinfo['port'];
  else
    $port = 80;

  $fp = @fsockopen($urlinfo['host'], 
                   $port,
                   $errno,
                   $errstr,
                   $timeout);

  # post the headers along with data
  $result = '';

  if($fp)
  {
       fwrite($fp, $header);
       fwrite($fp, $postdata);

       for ($i = 0; $i < count($data); $i++)
       {
         $data[$i]->send($fp);
       }

      while (!feof($fp))
         $result .= fgets($fp,1024);
      # close the server connection

      fclose($fp);

      # extract real data from result by removing headers
      $begin = strpos($result, "\r\n\r\n");
      $result = substr($result, $begin + 4);

      # see the result
      return $result;
  }
  else
  {
    error_log('Failed to connect to:' . $urlinfo['host'] . ':' . $port . ' - error #' . $errno . ': ' . $errstr);
    return false;
  }
}

?>
