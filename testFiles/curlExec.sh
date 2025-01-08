curl http://www.cs.cmu.edu/~dga/dga-headshot.jpg > headshotOut.jpg
curl -x 10.4.2.18:9098 http://www.cs.cmu.edu/~dga/dga-headshot.jpg > headshotOutProxy.jpg
curl -x 10.4.2.18:9098 http://www.cs.cmu.edu/~dga/dga-headshot.jpg > headshotOutProxy2.jpg
diff headshotOut.jpg headshotOutProxy.jpg
diff headshotOutProxy.jpg headshotOutProxy2.jpg
diff headshotOutProxy2.jpg headshotOut.jpg

echo "headshot diffs successfull"


curl http://www.cs.cmu.edu/~prs/bio.html > bioOut.txt
curl -x 10.4.2.18:9098 http://www.cs.cmu.edu/~prs/bio.html > bioOutProxy.txt
curl -x 10.4.2.18:9098 http://www.cs.cmu.edu/~prs/bio.html > bioOutProxy2.txt
diff bioOut.txt bioOutProxy.txt
diff bioOutProxy.txt bioOutProxy2.txt
diff bioOutProxy2.txt bioOut.txt

echo "bio diffs successfull"


curl http://www.cs.tufts.edu/comp/112/index.html > tuftsOut.txt
curl -x 10.4.2.18:9098 http://www.cs.tufts.edu/comp/112/index.html > tuftsOutProxy.txt
curl -x 10.4.2.18:9098 http://www.cs.tufts.edu/comp/112/index.html > tuftsOutProxy2.txt
diff tuftsOut.txt tuftsOutProxy.txt
diff tuftsOutProxy.txt tuftsOutProxy2.txt
diff tuftsOutProxy2.txt tuftsOut.txt

echo "tufts diffs successfull"


#ALL WITH -I FLAG
curl -i http://www.cs.cmu.edu/~dga/dga-headshot.jpg > headshotOutI.jpg
curl -i -x 10.4.2.18:9098 http://www.cs.cmu.edu/~dga/dga-headshot.jpg > headshotOutProxyI.jpg
curl -i -x 10.4.2.18:9098 http://www.cs.cmu.edu/~dga/dga-headshot.jpg > headshotOutProxy2I.jpg
diff headshotOutI.jpg headshotOutProxyI.jpg
diff headshotOutProxyI.jpg headshotOutProxy2I.jpg
diff headshotOutProxy2I.jpg headshotOutI.jpg

echo "headshot II diffs successfull"


curl -i http://www.cs.cmu.edu/~prs/bio.html > bioOutI.txt
curl -i -x 10.4.2.18:9098 http://www.cs.cmu.edu/~prs/bio.html > bioOutProxyI.txt
curl -i -x 10.4.2.18:9098 http://www.cs.cmu.edu/~prs/bio.html > bioOutProxy2I.txt
diff bioOutI.txt bioOutProxyI.txt
diff bioOutProxyI.txt bioOutProxy2I.txt
diff bioOutProxy2I.txt bioOutI.txt

echo "bio II diffs successfull"


curl -i http://www.cs.tufts.edu/comp/112/index.html > tuftsOutI.txt
curl -i -x 10.4.2.18:9098 http://www.cs.tufts.edu/comp/112/index.html > tuftsOutProxyI.txt
curl -i -x 10.4.2.18:9098 http://www.cs.tufts.edu/comp/112/index.html > tuftsOutProxy2I.txt
diff tuftsOutI.txt tuftsOutProxyI.txt
diff tuftsOutProxyI.txt tuftsOutProxy2I.txt
diff tuftsOutProxy2I.txt tuftsOutI.txt

echo "tufts II diffs successfull"