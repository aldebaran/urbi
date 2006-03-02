<?php
$file = "/var/www/html/svncount/buildnumber";


$num = file_get_contents($file);

$numinc = $num + 1;

file_put_contents($file, $numinc);
echo $numinc;
?>
