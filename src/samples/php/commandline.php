<?php

    function param($name)
    {
        global $argc;
        global $argv;

        $name = '--' . $name;
        for ($i = 0; $i < $argc; $i++)
        {
            if ($argv[$i] == $name && $i < $argc - 1)
                return $argv[$i + 1];
        }
        return null;
    }

    function flag($name)
    {
        global $argc;
        global $argv;

        $name = '--' . $name;
        for ($i = 0; $i < $argc; $i++)
        {
            if ($argv[$i] == $name)
                return true;
        }
        return false;
    }

?>