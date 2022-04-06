# Check if the app is running.
result=`ps aux | grep -i "opssat-segment.out" | grep -v "grep" | wc -l`

if [ $result -ge 1 ]
    then
        touch .stop
fi

result=`ps aux | grep -i "opssat-rf-segment.out" | grep -v "grep" | wc -l`

if [ $result -ge 1 ]
    then
        touch .stop
fi

result=`ps aux | grep -i "ranger" | grep -v "grep" | wc -l`

if [ $result -ge 1 ]
    then
        touch .stop
fi

# exit
exit 0  