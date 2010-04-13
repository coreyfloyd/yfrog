<?php
    require_once('commandline.php');
    require_once('yfrog.class.php');
    
    function usage()
    {
echo <<<EOT
Usage:
    yfrog-upload-and-post.php --username USERNAME --password PASSWORD --file FILE [--message MESSAGE] [--tags TAGS] [--public yes | --public no] [--source SOURCE]
EOT;
    }

    $username = param('username');
    $password = param('password');
    $file     = param('file');
    $tags     = param('tags');
    $public   = param('public');
    $message  = param('message');
    $source   = param('source');

    if (!$username || !$password || !$file)
    {
        usage();
        die();
    }

    if (!$source)
        $source = 'yfrog';

    $uploader = &new YfrogUploader();
    $response = $uploader->uploadAndPost($file, $message, $username, $password, $tags, $public == 'yes', YfrogUploader::YFROG_API_TIMEOUT, $source);
    if ($response['stat'])
        echo 'http://twitter.com/' . $username . '/status/' . $response['statusid'];
    else
        error_log('[' . $response['code'] . '] ' . $response['msg']);


?>