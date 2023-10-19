#!/bin/bash

cd ../game-logs-archive/s3-bucket
aws s3 sync s3://newcities-logs .
#aws s3 rm s3://newcities-logs --recursive
