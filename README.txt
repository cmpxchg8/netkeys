

    Netkeys v1.1 - Netopia Key Generator


# Background

  Netopia DSL modems were distributed throughout several countries by 
  Internet Service Providers some years ago (between 2004-2009 in Ireland)
  
  Netkeys demonstrates how default WEP keys are created and also a "Factory"
  password which can be used to bypass Admin lock.
    
    
# Magic mode
  
  While logged in with telnet access, there are a number of extra commands
  available if you type "magic" at the console.
  
  I've only found it useful when accessing 1 of the undocumented features.    
  There are other undocumented commands like this but not really that interesting.
    
  
# Backdoor / Factory access

  If the Admin account has a password set, there are still 2 other ways 
  to login using the "Factory" and "Backdoor" accounts.
  
  The "Backdoor" algorithm is documented in netkeys.c for reference but 
  incomplete due to a 64-bit challenge required to generate valid password
  which I was unable to understand. (no debugger access)
  
  Bear in mind, the "Backdoor" algorithm was documented using static 
  analysis and could be inaccurate.
    
  
# Manufacturing reset
  
  In "magic mode" is an undocumented command only available to the 
  "Factory" account which will erase the configuration set by the manufacturer.
  
  I'm unsure of this action being reversible so you could damage router.
  
  It removes all ISP branding and changed some hardware settings.
  To erase the configuration, login using "Factory" account and enter 
  "magic mode", then type "destroy_mfg_config"
  
  At this point, you'll be required to confirm with password which is "gat0TER"
  After completion, the router will reboot without ISP branding.
  
# Example output

  > netkeys 000FCC29C472

  Netkeys v1.1 - Netopia key generator.
  Copyright (c) 2007, 2008, 2010 Kevin Devine


  Serial Number : 19514482
  MAC           : 00:0F:CC:29:C4:72
  SSID          : 12345676
  Factory Pass  : agvlwzgv
  Backdoor Pass : ab52e9ffccb5915c

  [+] WEP - Manual (default)

  Encryption Key #1 b6382baf3ecaa196455fba0f93
  Encryption Key #2 50eae8d0c3cca988a78019ed76
  Encryption Key #3 005ad4de3a459c7546ca92d6a5
  Encryption Key #4 4d27563e02754f54230f166b6d

  [+] WEP - Automatic using "password"

  Encryption Key #1 4952d12624ade7120cb108a94b
  Encryption Key #2 7a1e11a38eaf31281df0c7d084
  Encryption Key #3 63f7e904aed4e9d345d90a6996
  Encryption Key #4 41f58add6025f9503863e30b40
  
