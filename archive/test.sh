#!/bin/bash

echo "Test 1"
echo "This is a test file 1" > file1.txt
echo "This is a test file 2" > file2.txt
echo "This is a test file 3" > file3.txt
ls *.txt
echo 

echo "Test 2"
./archive test_archive.arc -a file1.txt
./archive test_archive.arc -a file2.txt
./archive test_archive.arc -a file3.txt
ls *.txt *.arc
rm *.txt
echo

echo "Test 3"
./archive test_archive.arc -e
ls *.txt *.arc
rm *.txt
echo

echo "Test 4"
./archive test_archive.arc -d file2.txt
ls *.txt *.arc
rm *.txt
echo

echo "Test 5"
./archive test_archive.arc -e
ls *.txt *.arc
rm *.txt
echo