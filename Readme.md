
# Genearte your certifacte

set OPENSSL_CONF=..\..\..\openssl.cnf
set OPENSSL_EXE=C:\Libs\OpenSSL\x64-Release\bin\openssl.exe

%OPENSSL_EXE% req -x509 -nodes -days 365 -newkey rsa:1024 -keyout mycert.pem -out mycert.pem




Steps involved in SSL handshake (Courtesy:http://www.pierobon.org):

    The client sends the server the client's SSL version number, cipher settings, randomly generated data, and other information the server needs to communicate with the client using SSL.
    The server sends the client the server's SSL version number, cipher settings, randomly generated data, and other information the client needs to communicate with the server over SSL. The server also sends its own digital certificate and, if the client is requesting a server resource that requires client authentication, requests the client's digital certificate.
    The client uses the information sent by the server to authenticate the server. If the server cannot be authenticated, the user is warned of the problem that an encrypted and authenticated connection cannot be established. If the server can be successfully authenticated, the client proceeds.
    Using all data generated in the handshake so far, the client creates the premaster secret for the session, encrypts it with the server's public key (obtained from the server's digital certificate), and sends the encrypted premaster secret to the server.
    If the server has requested client authentication (an optional step in the handshake), the client also signs another piece of data that is unique to this handshake and known by both the client and server. In this case the client sends both the signed data and the client's own digital certificate to the server along with the encrypted premaster secret.
    If the server has requested client authentication, the server attempts to authenticate the client. If the client cannot be authenticated, the session is terminated. If the client can be successfully authenticated, the server uses its private key to decrypt the premaster secret, then performs a series of steps which the client also performs, starting from the same premaster secret to generate the master secret.
    Both the client and the server use the master secret to generate session keys which are symmetric keys used to encrypt and decrypt information exchanged during the SSL session and to verify its integrity.
    The client informs the server that future messages from the client will be encrypted with the session key. It then sends a separate encrypted message indicating that the client portion of the handshake is finished.
    The server sends a message to the client informing it that future messages from the server will be encrypted with the session key. It then sends a separate encrypted message indicating that the server portion of the handshake is finished.
    The SSL handshake is now complete, and the SSL session has begun. The client and the server use the session keys to encrypt and decrypt the data they send to each other and to validate its integrity.


