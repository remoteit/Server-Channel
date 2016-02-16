#!/bin/sh
#
# Copyright (C) 2016 Weaved Inc
#
# This is a notiication plugin for for bulk managment.  
#
# The calling formats are as follows 
#
# task_notify.sh 0 [taskid] [status]
#       returns 200 OK for   
#
# task_notify.sh 1
#       returns 200 OK for 
#
# Usage:  task_notify.sh <cmd> <status>
# 
# task_notify.sh [0/1/2/3] <$taskid> <$status>
#

#### Settings #####
VERSION=0.1
MODIFIED="February 6, 2016"
#
# Log to system log if set to 1
LOGGING=1
VERBOSE=0
#
#
apiMethod="https://"
apiVersion="/cors2"
apiServer="www.remot3.it"
apiServerCall="${apiMethod}${apiServer}${apiVersion}"

# Config curl path
CURL="curl"

#Default values
TMP="/tmp"
# Don't veryify SSL (-k) Silent (-s)
CURL_OPS=" -s"
DEBUG=1
#SRC=$1
#DST=$2
OUTPUT="$TMP/weaved_task_notify"

#
# Build API URLS GET API's                                                                                                          
API_TASK_UPDATE="${apiServerCall}/bulk/job/task/update/"    
API_TASK_DONE="${apiServerCall}/bulk/job/task/done/"
API_TASK_FAILED="${apiServerCall}/bulk/job/task/failed/"
API_DEVICE_STATUS_A="${apiServerCall}/bulk/service/status/a/"
API_DEVICE_STATUS_B="${apiServerCall}/bulk/service/status/b/"

#
# Helper Functions
#
#produces a unix timestamp to the output
utime()
{ 
    echo $(date +%s)
}

# produces a random number ($1 digits) to the output (supports upto 50 digits for now)
dev_random()
{
    local count=$1
    
    #defualt is 10 digits if none specified
    count=${1:-10};

    #1 to 50 digits supported    
    if [ "$count" -lt 1 ] || [ "$count" -ge 50 ]; then
        count=10;
    fi

    # uses /dev/urandom
    ret=$(cat /dev/urandom | tr -cd '0-9' | dd bs=1 count=$count 2>/dev/null)
    echo $ret
}

#
# JSON parse (very simplistic):  get value frome key $2 in buffer $1,  values or keys must not have the characters {}", and the key must not have : in them
#
jsonval()
{
    temp=`echo $1 | sed -e 's/[{}"]//g' -e 's/,/\n/g' | grep -w $2 | cut -d":" -f2-`
    echo ${temp##*|}
}


#
# urlencode $1
#
urlencode()
{
STR=$1
[ "${STR}x" == "x" ] && { STR="$(cat -)"; }

echo ${STR} | sed -e 's| |%20|g' \
-e 's|!|%21|g' \
-e 's|#|%23|g' \
-e 's|\$|%24|g' \
-e 's|%|%25|g' \
-e 's|&|%26|g' \
-e "s|'|%27|g" \
-e 's|(|%28|g' \
-e 's|)|%29|g' \
-e 's|*|%2A|g' \
-e 's|+|%2B|g' \
-e 's|,|%2C|g' \
-e 's|/|%2F|g' \
-e 's|:|%3A|g' \
-e 's|;|%3B|g' \
-e 's|=|%3D|g' \
-e 's|?|%3F|g' \
-e 's|@|%40|g' \
-e 's|\[|%5B|g' \
-e 's|]|%5D|g'
}

#
#
#
return_code()
{
    case $resp in
        "200")
            #Good Reponse
            echo "$resp OK"
            ;;
        "500")
            ret=$(jsonval "$(cat $OUTPUT)" "error") 
            echo "$resp $ret"
            ;;
        *)
            echo "$resp FAIL"
            ;;
    esac
}


#
# Print Usage
#
usage()
{
        if [ $LOGGING -gt 0 ]; then 
            logger "[task_notify.sh Called with bad format or -h]" 
        fi

        echo "Usage: $0 [-v (verbose)] [-v (maximum verbosity)] [-h (this message)] <CMD> <TASKID> <STATUS>" >&2
        echo "     [optional] Must specify <CMD> <TASKID> and <STATUS>" >&2
        echo "     CMD=0   --> update status " >&2
        echo "     CMD=1   --> completed status " >&2
        echo "     CMD=2   --> failed status " >&2
        echo "     CMD=3   --> status A " >&2
        echo "     CMD=4   --> status B " >&2
        echo "Version $VERSION Build $MODIFIED" >&2
        exit 1
}



###############################
# Main program starts here    #
###############################
#

################################################
# parse the flag options (and their arguments) #
################################################
while getopts vh OPT; do
    case "$OPT" in
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

# make sure we have at least 3 cmd, taskid and status
if [ $# -lt 3 ]; then
    usage
fi

# Parse off command
cmd=$1
shift

#Parse off taskid
task_id=$1
shift

status="$@"

if [ $LOGGING -gt 0 ]; then 
    logger "[task_notify.sh Called with cmd $cmd taskid $task_id value $status]" 
fi
if [ $VERBOSE -gt 0 ]; then
    echo "[task_notify.sh Called with cmd $cmd taskid $task_id value $status]"
fi



#
case $cmd in
    "0")
        #
        # Send Update 
        #
        URL="$API_TASK_UPDATE"
        data="{\"taskid\":\"${task_id}\",\"description\":\"${status}\"}"

        resp=$($CURL $CURL_OPS -w "%{http_code}\\n" -X POST -o "$OUTPUT" $URL -d "$data")

        if [ "$resp" -eq 200 ]; then
            # echo URL "return USERID"
            ret=$(jsonval "$(cat $OUTPUT)" "status") 
            echo "$resp $ret"
        else
            return_code $resp
        fi
    ;;

    "1")
        #
        # Task Done 
        #
        URL="$API_TASK_DONE"
        data="{\"taskid\":\"${task_id}\",\"description\":\"${status}\"}"

        resp=$($CURL $CURL_OPS -w "%{http_code}\\n" -X POST -o "$OUTPUT" $URL -d "$data")

        if [ "$resp" -eq "200" ]; then
            # echo URL "return USERID"
            ret=$(jsonval "$(cat $OUTPUT)" "status")
            echo "$resp $ret"
        else
            return_code $resp
        fi
    ;;
    "2")
        #
        # Task Failed 
        #
        URL="$API_TASK_DONE"
        data="{\"taskid\":\"${task_id}\",\"description\":\"${status}\"}"

        resp=$($CURL $CURL_OPS -w "%{http_code}\\n" -X POST -o "$OUTPUT" $URL -d "$data")

        if [ "$resp" -eq 200 ]; then
            # echo URL "return USERID"
            ret=$(jsonval "$(cat $OUTPUT)" "status")
            echo "$resp $ret"
        else
            return_code $resp
        fi        
    ;;
    "3" | "A" | "a")
        # device status A
        URL="$API_DEVICE_STATUS_A"
        data="{\"taskid\":\"${task_id}\",\"description\":\"${status}\"}"

        resp=$($CURL $CURL_OPS -w "%{http_code}\\n" -X POST -o "$OUTPUT" $URL -d "$data")

        if [ "$resp" -eq 200 ]; then
            # echo URL "return USERID"
            ret=$(jsonval "$(cat $OUTPUT)" "status")
            echo "$resp $ret"
        else
            return_code $resp
        fi        
    ;;
    "4" | "B" | "b")
        # device status 2
        URL="$API_DEVICE_STATUS_B"
        data="{\"taskid\":\"${task_id}\",\"description\":\"${status}\"}"

        resp=$($CURL $CURL_OPS -w "%{http_code}\\n" -X POST -o "$OUTPUT" $URL -d "$data")

        if [ "$resp" -eq 200 ]; then
            # echo URL "return USERID"
            ret=$(jsonval "$(cat $OUTPUT)" "status")
            echo "$resp $ret"
        else
            return_code $resp
        fi
    ;;
esac

# flush multiple returns
echo

