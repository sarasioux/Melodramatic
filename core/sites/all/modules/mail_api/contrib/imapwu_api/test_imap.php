<?
echo date("r");
echo "<hr>";

$username="denis";
$password="temp";
//$options="/notls";
//$connect_string="{localhost:143/notls}Drafts";
$connect_string="{localhost:143/notls}INBOX";

echo $username.':'.$password.'@'.$connect_string;
echo "<hr>";

$imap = imap_open($connect_string, $username, $password, $options);

$status = imap_status($imap, $connect_string, SA_ALL);
print_r(imap_errors());
echo $status;

$check = imap_check($imap);
dump($check);

$mailboxes = imap_getmailboxes($imap, $connect_string, "*");
print_r($mailboxes);

// Fetch an overview for all messages in INBOX

$result = imap_fetch_overview($imap,"1:{$check->Nmsgs}");
dump($result);

echo "<hr>";

$result = imap_fetchheader($imap, 4, FT_UID);
dump($result);


imap_close($imap);

function dump($data) {
  echo "<pre>";
  print_r($data);
  echo "</pre>";
}
?>
