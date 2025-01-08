./client HTTPRequest3.txt > clientReceived1.txt
./client HTTPRequest4.txt > clientReceived2.txt
./client HTTPRequest5.txt > clientReceived3.txt
./client HTTPRequest6.txt > clientReceived4.txt
./client HTTPRequest7.txt > clientReceived5.txt
./client HTTPRequest8.txt > clientReceived6.txt
diff serverResponse1.txt clientReceived1.txt
diff serverResponse2.txt clientReceived2.txt
diff serverResponse3.txt clientReceived3.txt
diff serverResponse4.txt clientReceived4.txt
diff serverResponse5.txt clientReceived5.txt
diff serverResponse6.txt clientReceived6.txt