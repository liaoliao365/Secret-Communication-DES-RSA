1. 使用Diffie-Hellman密码协商协议生成DES加密密钥,并分别存储在自己的数据文件中

    Alice:  ./bin/appAlice -g ./AliceData/alice.deskey
    Bob:    ./bin/appBob -g ./BobData/bob.deskey

2. Alice 加密 data.txt:

    ./bin/appAlice -e ./AliceData/alice.deskey ./AliceData/data.txt ./AliceData/data.enc

3. Alice把data.enc传给Bob, Bob将其存在./BobData/data.enc下

    Alice:  ./bin/appAlice -sec ./AliceData/data.enc
    Bob:    ./bin/appBob -sec ./BobData/data.enc

4. Bob 解密 data.enc :

    ./bin/appBob -d ./BobData/bob.deskey ./BobData/data.enc ./BobData/data_decrypted.txt

5. Alice生成公私钥对，将公钥发送给Bob

    Alice:  ./bin/appAlice -pub ./AliceData/alice.priv ./AliceData/alice.pub
    Bob:    ./bin/appBob -pub ./BobData/alice.pub 
    
6. Alice生成文件签名，发送给Bob，Bob检验签名

    Alice:  ./bin/appAlice -sig ./AliceData/data.enc ./AliceData/alice.priv
    Bob:    ./bin/appBob -sig ./AliceData/data.enc ./BobData/alice.pub 

    Alice:  ./bin/appAlice -sig ./AliceData/data.enc ./AliceData/alice.pub
    Bob:    ./bin/appBob -sig ./AliceData/data.enc ./AliceData/alice.priv 