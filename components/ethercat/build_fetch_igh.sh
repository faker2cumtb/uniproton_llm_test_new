echo "################# git clone ethercat #################"
pushd ../../../../src/net/
rm -rf ./ethercat
tar -zxf ../../packages/igh-ethercat/igh_master_20230720.tar.gz
patch -p1 -d ethercat <../../components/ethercat/igh.patch
popd
