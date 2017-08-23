#!/bin/sh

#How do I loop through only directories in bash?
for d in */ ; do
    echo "$d"
done
      # copy 
      cp -source -destination
      
      # create file 
      touch .bash_profile
      
      
  # list folder contents folder and rights
      ls -1
      
      # create file 
      # You can specify the file size in bytes (b), kilobytes (k), megabytes (m) or gigabytes (g)
      mkfile 1g test.abc
      
      
      # open opens files, directories and applications
      open /Applications/Safari.app/
      
      # copy and paste
      # These two commands let you copy and paste text from the command line. Of course, you could also just use your mouse—but the real power of pbcopy and pbpaste comes from the fact that they’re UNIX commands, and that means they benefit from piping, redirection, and the ability to be in scripts in conjunction with other commands
      ls ~ | pbcopy
      
      # will copy a list of files in your home directory to the OS X clipboard.
      pbcopy < blogpost.txt
      
      # find file
      mdfind -onlyin ~/Documents essay
      
      # copying large amounts of data as it can run within a Terminal window 
      # Adding -V, meaning verbose prints a line to the Terminal window for every file
      ditto -V /old/work/ /new/work/
            
      # stress test ma
      yes
      
      # view file system usage
      sudo fs_usage
      
      # View the Contents of Any File
      cat /path/to/file
      
  
       
       # Start a Simple HTTP Server in Any Folder
       python -m SimpleHTTPServer 8000
       
       # Run the Same Command Again
       !!
       sudo !!
       
       # download file without browser
       curl -O http://appldnld.apple.com/iTunes11/091-6058.20130605.Cw321/iTunes11.0.4.dmg
       
      
      # Continually Monitor the Output of a File
       tail -f /var/log/system.log
       
       # ip address
       ipconfig getifaddr en0			# internal
       curl ipecho.net/plain; echo	# external
       
       # network connectivity
       ping -c 10 www.apple.com
       
      
      # active processes
      top 
      
      # See A List of All The Commands You’ve Entered
      $ history
       
      # create bash exe
      chmod +x 
       		
      # screenshot
      # flags screencapture --help
      screencapture -C -M image.png
      screencapture -c -W
      screencapture -T 10 -P image.png
      screencapture -s -t pdf image.pdf
       
      # launchctl
      # launchctl lets you interact with the OS X init script system, launchd
      # Running launchctl list will show you what launch scripts are currently loaded. 
      sudo launchctl load -w
      
       
      # manual
      # The man command to bring up help manuals isn’t exclusive to OS X, nor is there much that’s new to say about it
      man
      
      
      # ssh-add for security keys
      # ssh -i keyfile.pem [server]
      ssh-add -k keyfile.pem
      ssh [server]
      
      # image processing (sips)
      # sips is an image processing tool and a native alternative to ImageMagick
      # http://www.leancrew.com/all-this/2014/05/a-little-sips/
      for file in *.jpeg; do sips -s format png $file --out $file.png
      
      
      # manupulate text docs
      # textutil uses Cocoa’s text engine to manipulate documents and convert them between various formats
      textutil -convert html article.doc
      textutil -cat rtf article1.doc article2.doc article3.doc
      
      # shows current director
      pwd