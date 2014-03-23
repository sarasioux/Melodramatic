Integration with Mandrill transactional emails, a service by the folks behind
MailChimp. Learn more about Mandrill and how to sign up on [their website](http://mandrill.com).

## Settings

### Mandrill Mail interface status
#### On
Routes all site emails through the STS API.

#### Test
Enables an alternate mail interface, TestingMailChimpMandrillMailSystem, that
displays a message and logs the event without sending any emails.

#### Off
Disables Mandrill and routes all email through the site's server.

### Email Options
* **Email from address:** The email address that emails should be sent from
* **From name:** The name to use for sending
* **_Input format_:** An optional input format to apply to the message body before sending emails

### Send Options
* **Track opens:** Toggles open tracking for messages
* **Track clicks:** Toggles click tracking for messages
* **Strip query string:** Strips the query string from URLs when aggregating tracked URL data

### Google Analytics
* **Domains:** One or more domains for which any matching URLs will
automatically have Google Analytics parameters appended to their query string.
Separate each domain with a comma.
* **Campaign:** The value to set for the utm_campaign tracking parameter. If
empty, the from address of the message will be used instead.

## Reports sub-module has not been ported to 6.x-1.x

### Dashboard
Displays charts that show volume and engagement, along with a tabular list of
URL interactions for the past 30 days.

### Account Summary
Shows account information, quotas, and all-time usage stats.
