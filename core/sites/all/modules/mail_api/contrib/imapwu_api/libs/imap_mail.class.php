<?php
/*
This is a wrapper around the IMAP protocol. It implements some functions which are not
native to IMAP but greately simply working with messages stored in IMAP.
*/
class imap_mail extends imap
{
	/*
	returns an array with message parts indexed and objects attached
	*/
	function get_map($mid)
	{
		$structure = $this -> get_structure($mid);

		if(!$structure->parts) {
			return FALSE;
		}

		return $this->create_part_array($structure);
	}

	function create_part_array($structure, $prefix="")
	{
		$part_array = array();

	    if (sizeof($structure->parts) > 0) {
	        foreach ($structure->parts as $count => $part) {
	            $this->add_part_to_array($part, $prefix.($count+1), $part_array);
	        }
	    }

   		return $part_array;
	}



	function add_part_to_array($obj, $partno, &$part_array)
	{

		if(!is_array($part_array) || empty($part_array)) $part_array=array();

		if ($obj->type == TYPEMESSAGE) {
	   $this->add_part_to_array($obj->parts[0], $partno.".", $part_array);
	  }
	  else {
	   if (sizeof($obj->parts) > 0) {
	     foreach ($obj->parts as $count => $p) {
	       $this->add_part_to_array($p, $partno.".".($count+1), $part_array);
	     }
	   }
	  }

	    $part_array[] = array('part_number' => $partno, 'part_object' => $obj);
	}

	/*
	returns a string that contains the plain text part of the message
	*/
	function get_plaintext($mid, $options="")
	{
		$map = $this->get_map($mid);

		//print_r($map);

		if(!$map) {
			// this is a simple plain text message
			$content = $this -> get_body($mid, 1, $options);

			$encoding = $this->get_plaintext_encoding($mid);

			//echo "encoding is: $encoding<br>\n";

			if($encoding=="base64") {
				$content = base64_decode($content);
			} else {
				$content = quoted_printable_decode($content);

			}


		} else {
			// extract the PLAIN part out of all parts

			foreach($map as $index=>$part_info) {

				if($part_info['part_object']->subtype == "PLAIN") {
					$content  = $this->get_part($mid, $part_info['part_number']);


					break;

				}

			}

		}

		if(!empty($content)) {
			return $content;
		} else {
			return FALSE;
		}
	}

	/*
	returns the charset for the plaintext part of the message
	*/
	function get_plaintext_charset($mid, $options="")
	{
		$map = $this->get_map($mid);



		if(!$map) {
			//echo "no map<br>\n";
			$header = $this->get_header($mid);

			$body = $this->get_body($mid,0);

			// see if there's a content type string
			if(preg_match("/Content-Type/", $body)) {
				//echo "there's content type<br>";

				// charset may or may not be enclosed in quotes
				preg_match("/charset=\"?(.*)\"?/", $body, $matches);


				$charset = $matches[1];
				return $charset;
			}

			/*
			echo "<pre>";
			print_r($body);
			echo "</pre>";
			*/
			return FALSE;

		} else {
			//echo "have map";
			// extract the PLAIN part out of all parts

			foreach($map as $index=>$part_info) {

				if($part_info['part_object']->subtype == "PLAIN" && is_array($part_info['part_object']->parameters)) {

					foreach($part_info['part_object']->parameters as $parameter_id=>$parameter_info) {
						if($parameter_info->attribute=="charset") {
							$charset = $parameter_info -> value;
							//echo "charset: $charset<br>";
							break;
						}
					}

				}

			}

		}

		if(!empty($charset)) {
			return $charset;
		} else {
			return FALSE;
		}
	}


	/*
	returns the encoding with which a particular message was encoded
	*/
	function get_plaintext_encoding($mid, $options="")
	{
		$body = $this->get_body($mid,0);

		// see if there's a content transfer string
		if(preg_match("/Content-Transfer-Encoding:/", $body)) {
			//echo "there's encoding<br>";

			preg_match("/Content-Transfer-Encoding:(.*)/", $body, $matches);

			$encoding = trim($matches[1]);
			return $encoding;
		}

		return FALSE;

	}

	/*
	returns an array with attachment details.
	this makes it very easy to work with attachments
	*/
	function get_attachment_overview($mid)
	{
		$map = $this->get_map($mid);

		// this message contains no attachemnts
		if(!$map) {
			return FALSE;
		}

		//die('we are here');

		foreach($map as $index=>$part_info) {
			if($part_info['part_object']->ifdisposition == 1) {
				$attachments[] = array(
				        'part_number' => $part_info['part_number'],
					'type' => $part_info['part_object']->subtype,
					'size' => $part_info['part_object']->bytes,
					'file' => $part_info['part_object']->dparameters[0]->value
				);
			}
		}

		return $attachments;
	}

	function get_attachment_files($mid) {

		// first get the overview
		$att_overview = $this -> get_attachment_overview($mid);


		// now return only those that have actual file names
		if(empty($att_overview)) return FALSE;
		if(sizeof($att_overview)<=0) return FALSE;

		foreach($att_overview as $att_id => $att_info) {
			if(!empty($att_info['file'])) {
				$rs[]=$att_info;
			}
		}

		return $rs;

	}

	/*
	this returns the part while decoding it
	*/
	function get_part($mid, $part)
	{

		//echo "getting part $part of $mid<br>\n";

		$map = $this -> get_map($mid);

		//print_r($map);

		if(!$map) {
			//echo "map is empty";
			return FALSE;
		}

		foreach($map as $index=>$part_info) {
			if($part_info['part_number']==$part) {



				if($part_info['part_object']->type ==0 || $part_info['part_object']->type =="") {




					if($part_info['part_object']->encoding==3) {
						$content = base64_decode($this->get_body($mid,$part));
					} elseif ($part_info['part_object']->encoding==4) {
						$content = quoted_printable_decode($this->get_body($mid,$part));
					} else {
						$content = $this->get_body($mid,$part);
					}

					// handle plain text data
					//$content = $this -> get_body($mid, $part);
				} else {


					// handle complex encoded data
					//echo "handling complex encoded data<br>";

					// handle base64 encoded data
					if($part_info['part_object']->encoding==3) {
						$content = base64_decode($this->get_body($mid,$part));
					}

					// handle quoted data
					if($part_info['part_object']->encoding==4) {
						$content = quoted_printable_decode($this->get_body($mid,$part));
					}

				}

				return $content;

			}
		}


		return FALSE;
	}




	/*
	switches the active folder by recreating the connection
	*/
	function active_folder($folder)
	{
		if(empty($folder)) return FALSE;
		if($this->folder == $folder) return;

		$this-> close();
		$this-> connect($this->username, $this->password, $this->host, $this->port, $folder, $this->options);

	}

	/*
	this returns an object of the part of a message
	*/
	function get_part_object($mid, $part)
	{
		if(!$this->mbox) return FALSE;

		$map = $this -> get_map($mid);

		foreach($map as $map_id=>$part_info) {
			if($part_info['part_number']==$part) {
				return $part_info['part_object'];
			}
		}

		return FALSE;
	}

	function get_part_mime_type($mid, $part) {
		$part = $this -> get_part_object($mid, $part);



	    switch(strtoupper($part -> subtype)) {
	      # Applications
	      case 'PDF':
	      case 'PS':
	      case 'POST-SCRIPT':
	        $mime = 'application/pdf';
	        $img = "$path/page_white_acrobat.png";
	        break;
	      case 'MSWORD':
	        $mime = 'application/msword';
	        $img = "$path/page_word.png";
	        break;
	      case 'PNG':
	      case 'GIF':
	      case 'BMP':
	      case 'JPG':
	      case 'JPEG':
	        $mime = "image/$type";
	        break;
	      case 'MP3':
	      case 'WMA':
	      case 'X-MS-WMA':
	      case 'AAC':
	        $mime = "audio/$type";
	        $img = "$path/sound.png";
	        break;
	      case 'MPEG':
	      case 'WMV':
	      case 'AVI':
	        $mime = "video/$type";
	        $img = "$path/film.png";
	        break;
	      default:
	        $mime = 'text/x-generic';
	        $img = "$path/application.png";
	    }

	    return $mime;
 	}


 	function mime_decode($data, $encoding)
 	{
		switch($encoding) {
			case ENC8BIT:
		        $data = imap_8bit($data);
		        $data = quoted_printable_decode($data);
		        break;
      		case ENCBASE64:
				$data = base64_decode(stripslashes($data));
				break;
			case ENCQUOTEDPRINTABLE:
        		$data = quoted_printable_decode($data);
       	 	break;
    	}
	    return $data;
  }

}
?>
