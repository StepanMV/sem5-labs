#!/bin/bash

echo "This is a test file 1" > file1.txt
echo "This is a test file 2" > file2.txt
echo "This is a test file 3" > file3.txt
ls *.txt *.arc

./archiver test_archive.arc -a file1.txt
./archiver test_archive.arc -a file2.txt
./archiver test_archive.arc -a file3.txt
ls *.txt *.arc
rm *.txt

./archiver test_archive.arc -e
ls *.txt *.arc
rm *.txt

./archiver test_archive.arc -d file2.txt
ls *.txt *.arc
rm *.txt

./archiver test_archive.arc -e
ls *.txt *.arc
rm *.txt