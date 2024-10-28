import requests
import time
import random

# Replace with your local IP and port
local_ip = 'http://172.18.100.109:80'


# request to join server
params = {'param': 'j60'}       # negotiates client speed. 
                                # 60 = amt of frames per movement. 
                                # for e.g. 2x speed, change the number to 30 and the sleep value to 0.5.
response = requests.post(local_ip,params=params)


params = {'param': 's'} # 'scan' request parameter
w_count = 0

while(True):
        
        time.sleep(1) # keep consist with join param message above
        
        # get scan for client
        scans = requests.get(local_ip,params=params)
        
        
        
        scan = ord(scans.text[0])-48 # converts ASCII char to 4 bit number, each bit corresponding
                                     # (in MSB order) to if up, down, left, or right are valid (1) or not (0)
                                     # for current client node.
                                     # NOTE: up, down, etc. are absolute cardinal directions, unrelated to current tank orientation.
        
        
        
        print(scan)

        # direction selection
        choices = []
        if(8&scan):
                choices.append('u')
        if(4&scan):
                choices.append('d')
        if(2&scan):
                choices.append('l')
        if(1&scan):
                choices.append('r')
                
        # chooses random movement from avaliable because writing SLAM coe would've taken too long
                
        rand = random.randrange(0, len(choices))
        
        move = requests.post(local_ip,params={'param':'m'+choices[rand]})
        
        # checks if client won, and if so, sends request to reset client to start position
        if(move.text and move.text[0]=='W'):
                print("w")
                w_count+=1
                print("new w count: "+str(w_count))
                requests.post(local_ip,params={'param':'r'})

       
        





