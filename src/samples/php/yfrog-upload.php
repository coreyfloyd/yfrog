<?php
    require_once('commandline.php');
    require_once('yfrog.class.php');
    
    function usage()
    {
echo <<<EOT
Usage:
    yfrog-upload.php --username USERNAME --password PASSWORD --file FILE [--tags TAGS] [--public yes | --public no]
EOT;
    }

    $username = param('username');
    $password = param('password');
    $file     = param('file');
    $tags     = param('tags');
    $public   = param('public');

    if (!$username || !$password || !$file)
    {
        usage();
        die();
    }

    $uploader = &new YfrogUploader();
    $response = $uploader->upload($file, $username, $password, $tags, $public == 'yes');
    if ($response['stat'])
        echo $response['mediaurl'];
    else
        error_log('[' . $response['code'] . '] ' . $response['msg']);


?>