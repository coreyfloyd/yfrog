<?php
    require_once('commandline.php');
    require_once('imageshack.class.php');
    
    function usage()
    {
echo <<<EOT
Usage:
    imageshack-upload.php --key DEVKEY --file FILE [--content-type CONTENT-TYPE] [--cookie COOKIE] [--remove-bar yes | --remove-bar no] [] [--tags TAGS] [--public yes | --public no] [--optsize SIZE]
EOT;
    }

    $key = param('key');
    if (!$key)
    {
        usage();
        die();
    }

    $file = param('file');
    if (!$file)
    {        
        usage();
        die();
    }

    $ct = param('content-type');

    $uploader = &new ImageShackUploader($key, param('cookie'));
    $public = param('public');
    if ($public)
        $public = $public != 'no';

    $response = $uploader->upload($file, 
                                  param('optsize'), 
                                  param('remove-bar') != 'no', 
                                  param('tags'),
                                  $public,
                                  $ct);

    print_r($response);

?>