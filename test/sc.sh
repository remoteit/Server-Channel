#!/bin/bash
#
#  Server Channel Test script to send a server channel message via weaved API 
#
#  sc.sh <-v> <-v> <devicename> <message to send>
#
#  <optional>  -v = verbose -v -v =maximum verbosity
#
#  will store info in ~/.weaved/
#
#  License See : https://github.com/weaved/server_channel
#
#  Weaved Inc : www.weaved.com
#
#  Author : https://github.com/lowerpower
#

# include shell script lib, must be in path or specify path here
source lib.sh

#set -x

#### Settings #####
VERSION=0.0.1
MODIFIED="Nov 12, 2015"
#
# Config Dir
#
WEAVED_DIR="$HOME/.weaved"
USER="$WEAVED_DIR/user"
ENDPOINTS="$WEAVED_DIR/endpoints"
AUTH="$WEAVED_DIR/auth"
CONNECTION_LOG="$WEAVED_DIR/log.$$.txt"
#
#
#
BIN_DIR=/usr/local/bin
EXE=weavedConnectd
#
# Save Auth in homedir
#
SAVE_AUTH=1
#
# use/store authhash instead of password (recommended)
#
USE_AUTHHASH=1
authtype=0 
#
#
apiMethod="https://"
apiVersion="/v22"
apiServer="api.weaved.com"
apiKey="WeavedDemoKey\$2015"
startPort=33000
#
# Global Vars if set will not ask for these
#
USERNAME=""
PASSWD=""
AHASH=""
#
# Other Globals
#
DEVICE_ADDRESS=""
DEVICE_STATE=""
LIST_ONLY=0
VERBOSE=0
DEBUG=0
PID=0;
TIMEIT=0
FAILTIME=10
#
# API URL's
#
loginURLpw="${apiMethod}${apiServer}${apiVersion}/api/user/login"
loginURLhash="${apiMethod}${apiServer}${apiVersion}/api/user/login/authhash"
logoutURL="${apiMethod}${apiServer}${apiVersion}/api/user/logout"
deviceListURL="${apiMethod}${apiServer}${apiVersion}/api/device/list/all"
sendServerSideURL="${apiMethod}${apiServer}${apiVersion}/api/device/send"
##### End Settings #####

#
# Built in manpage
#
manpage()
{
#
# Put manpage text here
#
read -d '' man_text << EOF

sc.sh - Server Channel Test Program
-----------------------------------

Sends a message to a device via the server channel


EOF
#
printf "\n%s\n\n\n" "$man_text"
}

#
# Print Usage
#
usage()
{
        echo "Usage: $0 [-v (verbose)] [-v (maximum verbosity)] [-l(ist devices only)] [-c(leanup)] [-r(eset to default)] [-m(an page)] [-h (this message)] <devicename> <message to send>" >&2
        echo "serverChannel Version $VERSION Build $MODIFIED" >&2
        exit 1 
}


#
# cleanup files that could affect normal operation if things went wrong
#
cleanup_files()
{
    if [ $VERBOSE -gt 0 ]; then
        printf "Cleaning up Weaved runtime files.  Removing auth file and active files.\n"
    fi   
    # reset auth
    rm -f $AUTH
    # reset active files
    rm -f ${WEAVED_DIR}/*.active
}
#
# Delete all the stuff in ~\.weaved to reset to default.  You may have to clean up your .ssh/known_hosts
# to get rid of ssh connection errors if your connections land on different ports
#
resetToDefault()
{
    if [ $VERBOSE -gt 0 ]; then
        printf "Resetting Weaved settings to default.\n"
    fi   
    rm -f ${WEAVED_DIR}/*
}


#
# Config directory creation if not exist and setup file permissions
#
create_config()
{
    umask 0077
    # create weaved directory
    if [ ! -d "$WEAVED_DIR" ]; then
        mkdir "$WEAVED_DIR" 
    fi
    # create files if they do not exist
    if [ ! -f "$ENDPOINTS" ] ; then
        touch "$ENDPOINTS"
    fi
    # cleanup old log files
    rm -f $WEAVED_DIR/*.txt
}
#
# Cleanup, this cleans up the files for the connection, and kills the P2P session if necessary
#
cleanup()
{
    if [ $DEBUG -eq 0 ]; then

        if [ $VERBOSE -gt 0 ]; then
            printf "Removing connection log.\n"
        fi    

        rm $CONNECTION_LOG
    else
        if [ $VERBOSE -gt 0 ]; then
            printf "Debug Mode, connection log is in $CONNECTION_LOG.\n"
        fi
    fi

    if [ $pid > 0 ]; then
        if [ $VERBOSE -gt 0 ]; then
            printf "Kill connection pid $pid.\n"
        fi
        kill $pid
    fi
    # cleanup port active file
    if [ -f "$WEAVED_DIR/${port}.active" ] ; then
        if [ $VERBOSE -gt 0 ]; then
            printf "Remove active flag file $WEAVED_DIR/${port}.active.\n"
        fi
        rm $WEAVED_DIR/${port}.active
    fi
}

#
# Control C trap
#
ctrap()
{
    if [ $VERBOSE -gt 0 ]; then
        echo "ctrl-c trap"
    fi

    cleanup
    exit 0;
}

#
# Find next unused port, returns the port to use, searches the $ENDPOINT cache
#
next_port()
{
    port=$startPort
    while [  1 ]; do
        # check if used
        grep "TPORT${port}" $ENDPOINTS > /dev/null 2>&1
        
        if [ $? = 1 ]; then
            # not found use this port
            break
        fi 

        let port=port+1
    done

    echo "$port"

}
#
# check_auth_cache, one line auth file, type is set to 0 for password and 1 for authash
# 
# Returns $username $password $type on success
#
check_auth_cache()
{
    # check for auth file
    if [ -e "$AUTH" ] ; then
        # Auth file exists, lets get it
        read -r line < "$AUTH"
        # Parse
        username=${line%%"|"*}
        password=${line##*"|"}
        t=${line#*"|"}
        authtype=${t%%"|"*}
        if [ $authtype -eq 1 ]; then
            ahash=$password
        fi
        return 1
    fi
    return 0
}

#
# Check Device Cache, return 1 if found and $port set
#
checkDeviceCache()
{
    port=0

    # check if device exists, if so get port
    dev_info=$(grep "|$device|" $ENDPOINTS)

    if [ $? = 0 ]; then
        #found grab port
        p=${dev_info%%"|"*}
        port=${p##*"TPORT"}
        #Get address DEVICE_ADDRESS
        DEVICE_ADDRESS=${dev_info##*"|"}
        return 1
    fi
    return 0
}

#
#parse devices
#
# fills device_array on return
#
parse_device()
{
    #parse devices data into lines, this old code is not MacOS mach compatible
    #lines=$(echo "$1" | sed  's/},{/}\n{/g' )
    #lines=$(echo "$in" | sed  's/},{/}\'$'\n''{/g' )
    #lines=$(echo "$1" | sed  's/},{/}|{/g' )
    #parse lines into array 
    #readarray -t device_array < <( echo "$lines" )
    # mac friendly replacement
    #device_array=( $(echo $lines | cut -d $'\n' -f1) )

    # New optimized code that works with MacOS
    lines=$(echo "$1" | sed  's/},{/}|{/g' )
    IFS='|'
    device_array=(  $lines )
}

#
# match_device 
#   match the passed device name to the array and return the index if found or 0 if not
#   if found device_state and device_address are set
#
match_device()
{
    # loop through the device array and match the device name
    for i in "${device_array[@]}"
    do
        # do whatever on $i
        #device_name=$(jsonval "$(echo -n "$i")" "devicealias") 
        device_name=$(jsonval "$i" "devicealias") 
   
        if [ "$device_name" = "$1" ]; then
            # Match echo out the UID/address
            #device_address=$(jsonval "$(echo -n "$i")" "deviceaddress")
            DEVICE_ADDRESS=$(jsonval "$i" "deviceaddress")
            DEVICE_STATE=$(jsonval "$i" "devicestate")
            #echo -n "$DEVICE_ADDRESS"
            return 1
        fi
    done

    #fail
    #echo -n "Not found"
    return 0
}

#
# Device List
#
display_devices()
{
    printf "%-25s | %-15s |  %-10s \n" "Device Name" "Device Type" "Device State"
    echo "--------------------------------------------------------------"
    # loop through the device array and match the device name
    for i in "${device_array[@]}"
    do
        # do whatever on $i
        device_name=$(jsonval "$i" "devicealias")
        device_state=$(jsonval "$i" "devicestate")
        device_service=$(jsonval "$i" "servicetitle")
        printf "%-25s | %-15s |  %-10s \n" $device_name $device_service $device_state
        #echo "$device_name : $device_service : $device_state"
    done
}


######### Begin Portal Login #########

getUserAndPassword() #get weaved user and password interactivly from user
{
    if [ "$USERNAME" != "" ]; then 
        username="$USERNAME"
    else
        printf "\n\n\n"
        printf "Please enter your Weaved username (email address): \n"
        read username
    fi

    if [ "$AHASH" != "" ]; then
        authtype=1
        ahash="$AHASH"
    else    

        if [ "$PASSWD" != "" ]; then
            password="$PASSWD"
        else
            printf "\nNow, please enter your password: \n"
            read  -s password
        fi
    fi
}


# userLogin
# returns 1 if logged in, token is set
# returns 0 if not logged in login error is set
userLogin () #Portal login function
{
    printf "Connecting...\n"
    
    if [ $authtype -eq 1 ]; then
        resp=$(curl -s -S -X GET -H "content-type:application/json" -H "apikey:${apiKey}" "$loginURLhash/$username/$ahash")
    else
        resp=$(curl -s -S -X GET -H "content-type:application/json" -H "apikey:${apiKey}" "$loginURLpw/$username/$password")
    fi

    status=$(jsonval "$(echo -n "$resp")" "status")

    login404=$(echo "$resp" | grep "404 Not Found" | sed 's/"//g')

    if [ "$login404" ]; then
        # 404 error
        loginerror="[404] API not found"
        return 0
    fi

    if [ "$status" == "true" ]; then
        # good, get token
        #token=$(jsonval "$(echo -n "$resp")" "token")
        token=$(jsonval "$resp" "token")
        date +"%s" > ~/.weaved_lastlogin
        # get atoken
        ahash=$(jsonval "$resp" "service_authhash")
        #echo "Got authhash >>$ahash"
        ret=1
    else
        loginerror=$(jsonval "$(echo -n "$resp")" "reason") 
        ret=0
    fi

    return "$ret"
}
 
######### End Portal Login #########

######### Device List ########
deviceList()
{
    resp=$(curl -s -S -X GET -H "content-type:application/json" -H "apikey:"${apiKey}"" -H "token:${token}" "$deviceListURL")
    echo $resp
}

######### Device List ########
sendServerChannel()
{

    base_command=$(echo -n "$command" | base64)

    resp=$(curl -s -S -X POST -H "content-type:application/json" -H "apikey:"${apiKey}"" -H "token:${token}" "$sendServerSideURL"  --data '{"deviceaddress":"'"$DEVICE_ADDRESS"'", "command":"'"$base_command"'"}')
    echo $resp

}


######### Wait for log event ########
#
# log_event, this funcion is the P2P connection manager for the session, it monitors the P2P engine for connection and
#   Failure Status.  Once a connection has been established, this script terminates
#
log_event()
{
    ret=0

    # create subshell and parse the logfile, modified to work around mac osx old bash
    # ( echo $BASHPID; tail -f $1) | while read LINE ; do
    (echo $(sh -c 'echo $PPID'); tail -f $1) | while read LINE ; do
        if [ -z $TPID ]; then
            TPID=$LINE # the first line is used to store the previous subshell PID
        else
            if [ $VERBOSE -gt 1 ]; then
                echo "log>>$LINE"
            fi

            case "$LINE" in
                *auto\ connect\ failed*)
                    kill -3 $TPID
                    echo "Cannot create a connection to device $device_name"
                    return 4
                    ;;
                *proxy\ startup\ failed*)
                    kill -3 $TPID            
                    echo "Port $port in use already, cannot bind to port"
                    return 1
                    ;;
                *Proxy\ started.*)
                    kill -3 $TPID 
                    echo "P2P tunnel connected on port $port" 
                    return 0
                ;;
                *usage:*)
                    kill -3 $TPID
                    echo "Error starting weavedConnectd daemon"
                    return 2
                    ;;
                *command\ not\ found*)
                    kill -3 $TPID
                    echo "weavedConnectd daemon not found In path"
                    return 3
                    ;;
                *state\ 5*)
                    printf "."
                    if [ $VERBOSE -gt 0 ]; then
                        echo "Connected to service, starting P2P tunnel"
                    fi
                    ;;
                *connection\ to\ peer\ closed\ or\ timed\ out*)
                    echo "Connection closed or timed out."
                    exit
                    ;;
                *!!status*)
                    printf "."
                    ;; 
            esac
        fi
    done

    ret=$?
    #echo "exited"
    return $ret 
}


###############################
# Main program starts here    #
###############################
#
# Create the config directory if not there
#
echo "Weaved sc.sh Version $VERSION $MODIFIED"
create_config

################################################
# parse the flag options (and their arguments) #
################################################
while getopts lvhmcr OPT; do
    case "$OPT" in
      c)
        cleanup_files
        exit 0
        ;;
      r)
        resetToDefault
        exit 0
        ;;
      m)
        manpage
        exit 0
        ;;
      l)
        LIST_ONLY=1 ;;
      v)
        VERBOSE=$((VERBOSE+1)) ;;
      h | [?])
        # got invalid option
        usage
        ;;
    esac
done

# get rid of the just-finished flag arguments
shift $(($OPTIND-1))

# make sure we have somthing to connect to
if [ $# -eq 0 ]; then
    usage
fi

in=$1

device=$1

#shift opps out
shift

command="$@"

#check cache to see if we have auth 
check_auth_cache
retval=$?
if [ "$retval" != 0 ]; then
    # Lets Login
    if [ $VERBOSE -gt 0 ]; then
        echo "Use stored Weaved credentials for user $username"
    fi
else
    getUserAndPassword
fi

    # Login the User (future check if already logged in with token or user exists in saved form)
    userLogin
    
    # check return value and exit if error
    retval=$?
    if [ "$retval" == 0 ]
    then
        echo $loginerror
        exit 255
    fi 

    if [ $VERBOSE -gt 0 ]; then
        echo "Logged in - get device list"
    fi

    #save auth
    if [ $SAVE_AUTH -gt 0 ]; then
        if [ ! -e "$AUTH" ] ; then
            if [ $VERBOSE -gt 0 ]; then
                echo "Saving Weaved credenials for $username"
            fi
            # Save either pw or hash depending on settings
            if [ $USE_AUTHHASH -eq 1 ]; then
                echo "${username}|1|${ahash}" > $AUTH 
            else
                echo "${username}|0|${[password}" > $AUTH 
            fi
        fi      
    fi

    # get device list
    dl_data=$(deviceList)

    # parse device list
    parse_device "$dl_data"


    if [ "$LIST_ONLY" -eq 1 ]; then
        # just display list only
        echo "Available devices"
        display_devices
        exit 0
    fi

    # Match Device passed to device list
    #address=$(match_device $device)
    match_device $device 

    retval=$?
    if [ "$retval" == 0 ]
    then
        echo "Device not found"
        exit 255
    fi

    # 
    # we have device and message, lets send
    #
    ret_data=$(sendServerChannel)

    echo "returned $ret_data"

    exit 0

