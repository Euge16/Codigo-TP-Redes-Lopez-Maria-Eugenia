#include <iostream>
#include <string>
#include <algorithm>
#include <WinSock2.h>
#include <fstream>
#include <sstream>


#pragma comment(lib, "ws2_32.lib")

using namespace std;

void mostrarMenu() {
    cout << "=======================================MENU PRINCIPAL=======================================" << endl;
    cout << "\n1. Traducir" << endl;
    cout << "2. Nueva traduccion" << endl;
    cout << "3. Usuarios" << endl;
    cout << "4. Ver registro de actividades" << endl;
    cout << "5. Cerrar sesion" << endl;
}

int main() {
    // Inicializar Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Error al inicializar Winsock" << endl;
        return 1;
    }

    // Crear un socket del cliente
    SOCKET clienteSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clienteSocket == INVALID_SOCKET) {
        cerr << "Error al crear el socket del cliente" << endl;
        WSACleanup();
        return 1;
    }

    int puerto;
    cout << "Ingrese el puerto del servidor: ";
    cin >> puerto;

    // Pedir al cliente nombre de usuario y contraseña
    string usuario;
    cout << "Ingrese su nombre de usuario: ";
    cin >> usuario;

    string contrasenia;
    cout << "Ingrese su contrasenia: ";
    cin >> contrasenia;

    // Configurar la dirección del servidor
    sockaddr_in direccionServidor;
    direccionServidor.sin_family = AF_INET;
    direccionServidor.sin_port = htons(puerto);
    direccionServidor.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Conectar al servidor
    if (connect(clienteSocket, (sockaddr*)&direccionServidor, sizeof(direccionServidor)) == SOCKET_ERROR) {
        cerr << "\nError al querer conectarse al servidor" << endl;
        closesocket(clienteSocket);
        WSACleanup();
        return 1;
    }


    string rol;

    // Enviar las credenciales al servidor
    string credenciales = usuario + " " + contrasenia;
    send(clienteSocket, credenciales.c_str(), credenciales.size(), 0);

    // Recibir la respuesta del servidor después de enviar las credenciales
    char respuestaServidor[1024];
    int bytesRecibidosRespuesta = recv(clienteSocket, respuestaServidor, sizeof(respuestaServidor), 0);
    if (bytesRecibidosRespuesta <= 0) {
        cerr << "Error al recibir la respuesta del servidor" << endl;
        closesocket(clienteSocket);
        WSACleanup();
        return 1;
    }

    respuestaServidor[bytesRecibidosRespuesta] = '\0';
    cout << "Respuesta del servidor: " << respuestaServidor << endl;

    bool autenticado = false;

    // Analizar la respuesta del servidor para autenticación y rol
    string respuestaCompleta(respuestaServidor);
    size_t separadorPos = respuestaCompleta.find('|');

    if (separadorPos != string::npos) {
        string accesoConcedido = respuestaCompleta.substr(0, separadorPos);
        rol = respuestaCompleta.substr(separadorPos + 1);

        if (accesoConcedido == "Acceso concedido") {
            autenticado = true; // El usuario está autenticado y tiene un rol válido
        }
    }

    while (autenticado) {
        mostrarMenu();

        int opcion;
        cout << "Seleccione una opcion: ";
        cin >> opcion;

        send(clienteSocket, reinterpret_cast<char*>(&opcion), sizeof(opcion), 0);

        switch (opcion) {
            case 1:
                 if (rol == "CONSULTA") {
                    string palabraEnIngles;
                    cin.ignore();
                    cout << "Ingresar palabra en Ingles: ";
                    getline(cin, palabraEnIngles);
                    send(clienteSocket, palabraEnIngles.c_str(), palabraEnIngles.size(), 0);

                    char bufferRespuesta[1024];
                    int bytesRecibidosRespuesta = recv(clienteSocket, bufferRespuesta, sizeof(bufferRespuesta), 0);

                    if (bytesRecibidosRespuesta <= 0) {
                        cerr << "Error al recibir datos del servidor despues de enviar palabraEnIngles" << endl;
                    } else {
                        bufferRespuesta[bytesRecibidosRespuesta] = '\0';
                        cout << bufferRespuesta << endl;
                    }
                }else{
                    cout << "Acceso no autorizado" << endl;

                }

                break;

            case 2:
                if (rol == "ADMIN") {
                    string nuevaTraduccion;
                    cin.ignore();
                    cout << "Ingresar nueva traduccion en el formato palabraEnIngles:palabraTraducida " << endl;
                    getline(cin, nuevaTraduccion);
                    transform(nuevaTraduccion.begin(), nuevaTraduccion.end(), nuevaTraduccion.begin(), ::tolower);
                    send(clienteSocket, nuevaTraduccion.c_str(), nuevaTraduccion.size(), 0);

                    char bufferRespuesta[1024];
                    int bytesRecibidosRespuesta = recv(clienteSocket, bufferRespuesta, sizeof(bufferRespuesta), 0);
                    if (bytesRecibidosRespuesta <= 0) {
                        cerr << "Error al recibir datos del servidor despues de enviar nuevaTraduccion" << endl;
                    } else {
                        bufferRespuesta[bytesRecibidosRespuesta] = '\0';
                        cout << bufferRespuesta << endl;

                    }
                }else{
                    cout << "Acceso no autorizado" << endl;

                }

                break;

            case 3:
                if (rol == "ADMIN") {
                    int menuDos;
                    bool regresarAlMenu = false;

                    while(!regresarAlMenu){
                        cout << "=======================================MENU SECUNDARIO=======================================" << endl;
                        cout << "\n1. Dar de alta" << endl;
                        cout << "2. Desbloquear" << endl;
                        cout << "3. Salir" << endl;
                        cout << "Seleccionar una opcion " << endl;
                        cin >> menuDos;
                        send(clienteSocket, reinterpret_cast<char*>(&menuDos), sizeof(menuDos), 0);


                            if (menuDos==1){
                                cin.ignore();
                                string nuevoUsuario;
                                string nuevaContrasenia;
                                cout << "Ingresar nuevo usuario: " << endl;
                                getline(cin, nuevoUsuario);

                                cout << "Ingresar contrasenia: " << endl;
                                getline(cin, nuevaContrasenia);

                                string creoNuevoUsuario = nuevoUsuario + "|" + nuevaContrasenia;
                                send(clienteSocket, creoNuevoUsuario.c_str(), creoNuevoUsuario.size(), 0);

                                // Recibir la respuesta del servidor
                                char bufferRespuesta[1024];
                                int bytesRecibidosRespuesta = recv(clienteSocket, bufferRespuesta, sizeof(bufferRespuesta), 0);
                                if (bytesRecibidosRespuesta <= 0) {
                                    cerr << "Error al recibir la respuesta del servidor al querer dar de alta" << endl;
                                } else {
                                    bufferRespuesta[bytesRecibidosRespuesta] = '\0';
                                    cout << bufferRespuesta << endl;
                                }

                            }
                            else if(menuDos == 2){
                                char bufferListaUsuarios[1024];
                                int bytesRecibidosListaUsuarios = recv(clienteSocket, bufferListaUsuarios, sizeof(bufferListaUsuarios), 0);
                                if (bytesRecibidosListaUsuarios <= 0) {
                                    cerr << "Error al recibir la lista de usuarios bloqueados" << endl;
                                } else {
                                    bufferListaUsuarios[bytesRecibidosListaUsuarios] = '\0';
                                    if (strcmp(bufferListaUsuarios, "No se encontraron usuarios bloqueados") == 0) {
                                        cout << "No hay usuarios bloqueados." << endl;
                                    } else {
                                        cout << "Usuarios bloqueados:\n" << bufferListaUsuarios << endl;

                                        // Solicitar al cliente el usuario a desbloquear
                                        string usuarioDesbloquear;
                                        cout << "Ingrese el nombre de usuario a desbloquear: ";

                                        cin >> usuarioDesbloquear;



                                        send(clienteSocket, usuarioDesbloquear.c_str(), usuarioDesbloquear.size(), 0);

                                        // Recibir la respuesta del servidor
                                        char bufferRespuesta[1024];
                                        int bytesRecibidosRespuesta = recv(clienteSocket, bufferRespuesta, sizeof(bufferRespuesta), 0);
                                        if (bytesRecibidosRespuesta <= 0) {
                                            cerr << "Error al recibir la respuesta del servidor después de desbloquear" << endl;
                                        } else {
                                            bufferRespuesta[bytesRecibidosRespuesta] = '\0';
                                            cout << bufferRespuesta << endl;
                                        }
                                    }
                                    }
                            }else if(menuDos == 3){
                                break;
                            }


                            else{
                                cout << "Opcion no valida. Por favor, seleccione una opcion valida." << endl;
                            }
                }
                }

                else{
                    cout << "Acceso no autorizado" << endl;

                }

                break;

            case 4:
                if (rol == "ADMIN") {

                    while (true) {
                        char buffer[1024];
                        int bytesRecibidos = recv(clienteSocket, buffer, sizeof(buffer), 0);

                        buffer[bytesRecibidos] = '\0';

                        istringstream stream(buffer);
                        string linea;
                        while (getline(stream, linea)) {
                            cout << linea << endl;
                        }


                        if (strcmp(buffer, "FIN_REGISTRO") == 0) {
                            break;
                        }


                    }
                }else {
                    cout << "Acceso no autorizado" << endl;
                }
                break;


            case 5:

                closesocket(clienteSocket);
                WSACleanup();
                autenticado = false;

                break;

            default:
                cout << "Opcion no valida. Intente de nuevo." << endl;
                break;
        }

    }

    // Cerrar la conexión con el servidor
    closesocket(clienteSocket);

    // Finalizar Winsock
    WSACleanup();

    return 0;
}









