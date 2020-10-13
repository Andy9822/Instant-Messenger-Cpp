Para rodar o projeto:

Build:
    - Entre no diretório Instant-Messenger/
    - Rodar make

Servidor:
    - Entre no diretório Instant-Messenger/
    - Após estar buildado rode: ./build/server.app <porta> <numeroDeMensagensASeremGravadasNoHistorico>
        Por exemplo: ./build/server.app 4040 4

Cliente:
    - Entre no diretório Instant-Messenger/
    - Após estar buildado e ter um server rodando, rode: ./build/client.app <nomeUsuario> <nomeSala> <IP> <port>
        Exemplo: ./build/client.app JohnnyUser1 SampleRoom1 127.0.0.1 4040