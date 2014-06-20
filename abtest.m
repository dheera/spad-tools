% test if ascii and binary file outputs are the same

data1 = load('foo.ascii.out');
fid = fopen('foo.binary.out');

bindata = fread(fid,'uint64');
fclose(fid);

data2 = [ bitshift(bindata,-32) bitshift(bitand(bindata,2^32-2^16),-16) bitand(bindata,2^10-2^0)];

find(sum((data2==data1)')<3)
