<?php
    require_once('commandline.php');
    require_once('imageshack.class.php');
    
    function usage()
    {
echo <<<EOT
Usage:
    imageshack-transload.php --key DEVKEY --url URL [--cookie COOKIE] [--remove-bar yes | --remove-bar no] [] [--tags TAGS] [--public yes | --public no] [--optsize SIZE]
EOT;
    }

    $key = param('key');
    if (!$key)
    {
        usage();
        die();
    }

    $url = param('url');
    if (!$url)
    {        
        usage();
        die();
    }

    $uploader = &new ImageShackUploader($key, param('cookie'));
    $public = param('public');
    if ($public)
        $public = $public != 'no';

    $response = $uploader->transload($url, 
                                     param('optsize'), 
                                     param('remove-bar') != 'no', 
                                     param('tags'),
                                     $public);
    print_r($response);

?>