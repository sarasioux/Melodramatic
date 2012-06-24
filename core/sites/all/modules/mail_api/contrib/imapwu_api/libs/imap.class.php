<?php
/*
This class implements a basic IMAP protocol. It does not add any additional functionality
*/

class imap
{
	var $username;
	var $password;
	var $host;
	var $port;
	var $folder;
	var $options;
	var $connect_string;
	var $mbox;


	/*
	connects to the IMAP server
	*/
	function connect($username, $password, $host, $port=143, $folder="", $options)
	{

		//echo "<p>connecting to $username:$password@$host:$port/$folder</p>";

		if(empty($username) || empty($password) || empty($host)) return FALSE;

		$this -> username = $username;
		$this -> password = $password;
		$this -> host = $host;
		$this -> port = $port;
		$this -> folder = $folder;
		$this -> options = $options;

		$this -> connect_string = '{'.$host.':'.$port.$this->options.'}'.$folder;

		//echo $this->connect_string."<br>";


		if($this -> mbox = imap_open($this->connect_string, $username, $password)) {
			return TRUE;
		} else {
			return FALSE;
		}

	}

  /**
  * Implement Imap2 Search capability in the system
  * @query: the string with which to carry out the search
  * @criteria: array of flags to search using the query i.e. BODY, TO, FROM, SUBJECT
  * @flags: array of flags that are by themself with no string associated i.e. \\SEEN, \\UNSEEN
  */
  function search($query, $criteria, $flags) {
    if (!$this->mbox) return FALSE;

    // one of the variables needs to have data to perform a search
    if (empty($query) && sizeof($flags)<=0) return FALSE;

    $search = '';

    if (!empty($query)) {
      // str replace is so that any special characters are taken care of
      $query = "\"" . str_replace("\"", "", $query) . "\"";
      foreach ($criteria as $c) {
        $search .= $c . " " . $query . " ";
      }
    }

    foreach ($flags as $flag_id=>$flag) {
      $search .= $flag . " ";
    }
      

    // echo "SEARCH STRING: $search<br>\n";

    // get results set from the imap search
    $rs = imap_search($this->mbox, $search);

    if(empty($rs)) return FALSE;

    return array_unique($rs);
  }

	/*
	implements stock imap_status function - returns a brief overview of new messages in a mailbox
	*/
	function get_status($folder)
	{
		if(!$this->mbox) return FALSE;

		$root_connect_string = substr($this->connect_string, 0, strpos($this->connect_string, "}")+1);

       	return imap_status ($this->mbox, $root_connect_string.$folder, SA_ALL);

	}


	/*
	implements the stock imap_sort function
	*/
	function get_sort($criteria, $reverse)
	{
		if(!$this->mbox) return FALSE;

		return imap_sort($this->mbox, $criteria, $reverse);
	}


	function get_mbox_info()
	{
		if(!$this->mbox) return FALSE;

    	return imap_mailboxmsginfo($this->mbox);
  	}


  	/*
  	returns an array of objects with folders
  	*/
	function get_folders($filter = "*")
	{
		if(!$this->mbox) return FALSE;

    	$folders = imap_getmailboxes($this->mbox, '{' . $this->host .':' . $this->port . '}', $filter);
    	return $folders;
	}

	/*
	returns an array of objects containing subscribed folders
	*/
	function get_subscriptions($filter = "*")
	{
		if(!$this->mbox) return FALSE;

    	$folders = imap_getsubscribed($this->mbox, '{' . $this->host .':' . $this->port . '}', $filter);
    	return $folders;
  	}

  	/*
  	returns an array of objects containg headers in the mailbox
  	*/
  	function get_headers()
  	{
  		if(!$this->mbox) return FALSE;

  		$headers = imap_headers($this->mbox);
  		return $headers;
  	}


 	function get_header_full($mid)
 	{
    if(empty($mid)) return FALSE;
    if(!$this->mbox) return FALSE;
     		
    
    
 		$rs = imap_fetchheader($this->mbox, $mid);
 		$raw = explode("\n", $rs);
 		foreach($raw as $index=>$line) {
 			if(trim($line)=="") continue;
 	    list($variable, $value) = split(":", $line, 2);
 	    $set[trim($variable)]=trim($value);
 		}
 		
 		
 		$obj = $this -> get_header($mid);
 		
 		$obj -> full_toaddress = $set["To"];
 		$obj -> full_ccaddress = $set["Cc"];
 		//__webmail_plus_dump($set);
 	  /*	
 		$obj = new stdClass();
 		$obj -> date = $set["Date"];
 		$obj -> Date = $set["Date"];
 		$obj -> MailDate = $set["Date"];
 		$obj -> subject = $set["Subject"];
 		$obj -> Subject = $set["Subject"];
 		$obj -> toaddress = $set["To"];
 		$obj -> fromaddress = $set[""]
 		$obj -> reply_toaddress =
 		[senderaddress]
    */
 		
 		return $obj;	
 	} 
  	
  	/*
  	returns a header for an individual message in form of an object which contains a header
  	*/
	function get_header($mid)
	{
		if(empty($mid)) return FALSE;
		if(!$this->mbox) return FALSE;

		
		$rs = imap_headerinfo($this->mbox, $mid);
		
		//print_r($rs);
		
		return $rs;
		
	}

	/*
	returns an array of objects with headers for all messages in the folder
	*/
  	function get_overview()
  	{
  		if(!$this->mbox) return FALSE;

  		$check = imap_check($this->mbox);
		$messages = $check -> Nmsgs;

		if ($messages > 0) {
			$overview = imap_fetch_overview($this->mbox, "1:".$messages."", 0);
			// this makes the array index identical to msgno, makes life easier
			foreach ($overview as $index => $record) {
				$rs[$record->msgno] = $record;
			}
			return $rs;
		} else {
			return FALSE;
		}
  	}

    /*
    Copies a message to a different folder
    */
    function copy($mid, $destination) {
    	if (!$this->mbox) return FALSE;
		return imap_mail_copy($this->mbox, $mid, $destination);
    }

    /*
    implements IMAP check function
    */
    function check() {
    	if (!$this->mbox) return FALSE;

    	return imap_check($this->mbox);
    }




  	/*
  	moves a message to a different folder
  	*/

	function move($mid, $destination) {
		if(!$this->mbox) return FALSE;

		return imap_mail_move($this->mbox, $mid, $destination);
	}


  	function delete($mid) {
		if (!$this->mbox) return FALSE;

		//die("removing $this->mbox, $mid");

  		return imap_delete($this->mbox, $mid);
  	}

    /*
    returns the structure of the IMAP message in form of an object
    */
    function get_structure($mid, $options="") {
 		if(!$this->mbox) return FALSE;
       	return imap_fetchstructure($this->mbox, $mid, $options);
    }

    /*
    returns a part of a message in form of an object
    */
    function get_body($mid, $part, $options="") {
       	if(!$this->mbox) return FALSE;
       	return imap_fetchbody($this->mbox, $mid, $part, $options);
    }

    function get_message($mid) {
       	$structure = $this -> get_structure($mid);
       	$parts = $structure -> parts;
       	foreach($parts as $key => $part) {}
      	return $structure;
    }

    function get_raw_message($mid) {
       	if(!$this->mbox) return FALSE;
      	return imap_body($this->mbox, $mid, SE_UID);
    }

    function add_folder($folder) {
    	if(!$this->mbox) return FALSE;
	    if(empty($folder)) return;

	    // trim the connect string
      $root_connect_string = substr($this->connect_string, 0, strpos($this->connect_string, "}")+1);
      
      return imap_createmailbox($this->mbox, $root_connect_string.$folder);
    }

    function rename_folder($oldname, $newname) {
        if (!$this->mbox) return FALSE;
        if (empty($oldname) || empty($newname)) return FALSE;

        $root_connect_string = substr($this->connect_string, 0, strpos($this->connect_string, "}")+1);
        return imap_renamemailbox($this->mbox, $root_connect_string . $oldname, $root_connect_string . $newname);
    }

    function delete_folder($folder) {
    	if(!$this->mbox) return FALSE;
		if(empty($folder)) return;

		$root_connect_string = substr($this->connect_string, 0, strpos($this->connect_string, "}")+1);
		return imap_deletemailbox($this->mbox, $root_connect_string.$folder);
    }

    function get_mime_type(&$structure) {
      	$primary_mime_type = array("TEXT", "MULTIPART","MESSAGE", "APPLICATION", "AUDIO","IMAGE", "VIDEO", "OTHER");

       	if($structure->subtype) {
         	return $primary_mime_type[(int) $structure->type] . '/' .$structure->subtype;
   		}
  	 	return "TEXT/PLAIN";
    }

    function expunge() {
        if(!$this->mbox) return FALSE;
       	return imap_expunge($this->mbox);
    }

    /*
    sets a flag on a message
    */
    function set_flag($mid,$flags) {
       	if(!$this->mbox) return FALSE;
       	if(empty($flags)) return FALSE;

       	return imap_setflag_full($this->mbox, $mid, $flags);
    }

    /*
    clears a flag on a message
    */
    function clear_flag($mid,$flags) {
       	if(!$this->mbox) return FALSE;
       	if(empty($flags)) return FALSE;

       	return imap_clearflag_full($this->mbox, $mid, $flags);
    }

    /*
     puts a message in a folder
     this is used to store a sent message in the Sent folder
    */
    function append($folder, $message) {
    	if(!$this->mbox) return FALSE;
       	if(empty($folder)) return;
      	// trim the connect string

      	//echo "appending $message";

      	$root_connect_string = substr($this->connect_string, 0, strpos($this->connect_string, "}")+1);
       	return imap_append($this->mbox, $root_connect_string.$folder, $message);
    }

    /*
    disconnects from IMAP server
    */
    function close() {
       	if($this->open) {
	  		imap_close($this->mbox);
	  		return TRUE;
    	}
       	return FALSE;
    }

    function __has_prefix() {
       	if(preg_match("/\}[\w]/", $this->connect_string)) {
          return TRUE;
		} else {
		  return FALSE;
		}
    }


    function get_errors() {
    	return imap_errors();
    }

    function set_timeout($type=1,$seconds=0) {
    	imap_timeout($type, $seconds);
    }
}
?>
