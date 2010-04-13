<?php
    require_once('commandline.php');
    require_once('yfrog.class.php');
    
    function usage()
    {
echo <<<EOT
Usage:
    yfrog-transload-and-post.php --username USERNAME --password PASSWORD --url URL [--message MESSAGE] [--tags TAGS] [--public yes | --public no]
EOT;
    }

    $username = param('username');
    $password = param('password');
    $url      = param('url');
    $tags     = param('tags');
    $public   = param('public');
    $message  = param('message');
    $source   = param('source');

    if (!$username || !$password || !$url)
    {
        usage();
        die();
    }

    if (!$source)
        $source = 'yfrog';

    $uploader = &new YfrogUploader();
    $response = $uploader->transloadAndPost($url, $message, $username, $password, $tags, $public == 'yes', YfrogUploader::YFROG_API_TIMEOUT, $source);
    if ($response['stat'])
        echo 'http://twitter.com/' . $username . '/status/' . $response['statusid'];
    else
        error_log('[' . $response['code'] . '] ' . $response['msg']);


?>