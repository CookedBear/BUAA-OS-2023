mkdir mydir
chmod 777 mydir

touch myfile.txt
echo 2023 > myfile.txt
mv moveme mydir/moveme
cp copyme mydir/copied
cat readme
gcc bad.c 2> err.txt

n=10

one=1
if [ $# -ge $one ]
then
	n=$1
fi

mkdir gen
i=1
while [ $i -le $n ]
do
	touch gen/$i.txt
	let i=$i+1
done

