#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <sstream>
#include <ctime>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

struct Credencial {
    string usuario;
    string contrasenia;
    string rol;
    int intentos;
};

vector<Credencial> cargarCredenciales(const string& archivo) {
    vector<Credencial> credenciales;
    ifstream archivoCredenciales(archivo);

    if (archivoCredenciales.is_open()) {
        string linea;
        while (getline(archivoCredenciales, linea)) {
            stringstream ss(linea);
            Credencial credencial;
            getline(ss, credencial.usuario, '|');
            getline(ss, credencial.contrasenia, '|');
            getline(ss, credencial.rol, '|');
            ss >> credencial.intentos;
            credenciales.push_back(credencial);
        }
        archivoCredenciales.close();
    }

    return credenciales;
}

bool verificarCredenciales(const string& username, const string& password, const vector<Credencial>& credenciales) {
    for (const Credencial& credencial : credenciales) {
        if (credencial.usuario == username && credencial.contrasenia == password) {
            return true;
        }
    }
    return false;
}

bool verificarUsuarioYContrasenia(const string& nuevoUsuario, const string& nuevaContrasenia){
    if (nuevoUsuario.empty() || nuevaContrasenia.empty() || nuevoUsuario.find_first_not_of(' ') == string::npos || nuevaContrasenia.find_first_not_of(' ') == string::npos || nuevoUsuario.find('\n') != string::npos || nuevaContrasenia.find('\n') != string::npos) {
        return true;
    }
    return false;
}

bool usuarioExiste(vector<Credencial>& credenciales, const string& nuevoUsuario){
    bool usuarioE= false;
    for(const Credencial& credencial: credenciales){
        if(credencial.usuario == nuevoUsuario){
            usuarioE = true;
        }
    }
    return usuarioE;
}


void darDeAlta(vector<Credencial>& credenciales, const string& nuevoUsuario, const string& nuevaContrasenia ){
    if(usuarioExiste(credenciales,nuevoUsuario)!=true){
        Credencial nuevaCredencial;
        nuevaCredencial.usuario = nuevoUsuario;
        nuevaCredencial.contrasenia = nuevaContrasenia;
        nuevaCredencial.rol = "CONSULTA";
        nuevaCredencial.intentos = 0;
        credenciales.push_back(nuevaCredencial);
    }
}

vector<string> listarUsuariosBloqueados(const vector<Credencial>& credenciales) {
    vector<string> usuariosBloqueados;
    for (const Credencial& credencial : credenciales) {
        if (credencial.intentos == 3) {
            usuariosBloqueados.push_back(credencial.usuario);
        }
    }
    return usuariosBloqueados;
}


struct Traduccion {
    string palabraEnIngles;
    string palabraTraducida;
};

vector<Traduccion> cargarTraducciones(const string& archivo) {
    vector<Traduccion> traducciones;
    ifstream archivoTraducciones(archivo);

    if (archivoTraducciones.is_open()) {
        string linea;
        while (getline(archivoTraducciones, linea)) {
            stringstream ss(linea);
            Traduccion traduccion;
            getline(ss, traduccion.palabraEnIngles, ':');
            getline(ss, traduccion.palabraTraducida);
            traducciones.push_back(traduccion);
        }
        archivoTraducciones.close();
    }
    return traducciones;
}

string traducirPalabra(string palabraEnIngles, const vector<Traduccion>& traducciones) {
    string traduccion;
    transform(palabraEnIngles.begin(), palabraEnIngles.end(), palabraEnIngles.begin(), ::tolower);
    for (const Traduccion& t : traducciones) {
        if (t.palabraEnIngles == palabraEnIngles) {
            traduccion = t.palabraTraducida;
            break;
        }
    }
    return traduccion;
}

bool traduccionExiste(const vector<Traduccion>& traducciones, const string& nuevaTraduccion) {
    size_t separadorPos = nuevaTraduccion.find(":");
    string palabraEnIngles = nuevaTraduccion.substr(0, separadorPos);
    string palabraTraducida = nuevaTraduccion.substr(separadorPos + 1);

    for (const Traduccion& t : traducciones) {
        if (t.palabraEnIngles == palabraEnIngles) {
            return true;
        }
    }
    return false;
}

bool agregarNuevaTraduccion(const string& nuevaTraduccion, vector<Traduccion>& traducciones) {

    size_t separadorPos = nuevaTraduccion.find(":");
    if (separadorPos == string::npos) {
        return false;
    }

    string palabraEnIngles = nuevaTraduccion.substr(0, separadorPos);
    string palabraTraducida = nuevaTraduccion.substr(separadorPos + 1);


    if (palabraEnIngles.empty() || palabraTraducida.empty()) {
        return false;
    }

    for (const Traduccion& t : traducciones) {
        if (t.palabraEnIngles == palabraEnIngles) {
            return false;
        }
    }

    Traduccion nuevaT;
    nuevaT.palabraEnIngles = palabraEnIngles;
    nuevaT.palabraTraducida = palabraTraducida;
    traducciones.push_back(nuevaT);

    /
    ofstream archivoTraducciones("traducciones.txt", ios::app);
    if (archivoTraducciones.is_open()) {
        archivoTraducciones << palabraEnIngles << ":" << palabraTraducida << endl;
        archivoTraducciones.close();
    } else {
        return false;
    }
    return true;
}


void escribirRegistro(const string& mensaje) {
    ofstream archivoRegistro("server.log", ios::app); // Abre el archivo en modo de anexar
    if (archivoRegistro.is_open()) {
        time_t tiempoActual = time(nullptr);
        tm* tiempoInfo = localtime(&tiempoActual);
        char fechaHora[20];
        strftime(fechaHora, sizeof(fechaHora), "%Y-%m-%d_%H:%M", tiempoInfo);
        archivoRegistro << fechaHora << ": " << mensaje << endl;
        archivoRegistro.close();
    }
}

 int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Error al inicializar Winsock" << endl;
        return 1;
    }
    // Crear el socket del servidor
    SOCKET servidorSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (servidorSocket == INVALID_SOCKET) {
        cerr << "Error al crear el socket" << endl;
        WSACleanup();
        return 1;
    }
    int puerto;
    cout << "Ingresar puerto en el que va a iniciar el servidor: ";
    cin >> puerto;
    cin.ignore();
    // Configurar la información del servidor
    sockaddr_in servidorAddr;
    servidorAddr.sin_family = AF_INET;
    servidorAddr.sin_port = htons(puerto);
    servidorAddr.sin_addr.s_addr = INADDR_ANY;
    // Vincular el socket al puerto
    if (bind(servidorSocket, (struct sockaddr*)&servidorAddr, sizeof(servidorAddr)) == SOCKET_ERROR) {
        cerr << "Error al vincular el socket al puerto" << endl;
        closesocket(servidorSocket);
        WSACleanup();
        return 1;
    }
    // Escuchar las conexiones entrantes
    if (listen(servidorSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Error al escuchar conexiones entrantes" << endl;
        closesocket(servidorSocket);
        WSACleanup();
        return 1;
    }


    cout << "Servidor en espera de conexiones entrantes..." << endl;

    escribirRegistro("=======Inicia Servidor=======");
    // Abrir el archivo credenciales.txt
    fstream archivoCredenciales("credenciales.txt", ios::in | ios::out );
    if (!archivoCredenciales.is_open()) {
        cerr << "Error al abrir el archivo credenciales.txt" << endl;
        closesocket(servidorSocket);
        WSACleanup();
        return 1;
    }

    ofstream archivoTraducciones("traducciones.txt", ios::app);

    vector<Credencial> credenciales = cargarCredenciales("credenciales.txt");

    vector<Traduccion> traducciones = cargarTraducciones("traducciones.txt");



    while (true) {

        // Aceptar una conexión entrante
        SOCKET clienteSocket = accept(servidorSocket, NULL, NULL);
        if (clienteSocket == INVALID_SOCKET) {
            cerr << "Error al aceptar la conexion" << endl;
            continue;
        }
        // Recibir las credenciales del cliente
        char bufferCredenciales[1024];
        int bytesRecibidosCredenciales = recv(clienteSocket, bufferCredenciales, sizeof(bufferCredenciales), 0);
        if (bytesRecibidosCredenciales <= 0) {
            cerr << "Error al recibir los datos del cliente" << endl;
            closesocket(clienteSocket);
            continue;
        }

        bufferCredenciales[bytesRecibidosCredenciales] = '\0';
        stringstream  credencialesCliente(bufferCredenciales);
        string usuario, contrasenia, rol;
        credencialesCliente >> usuario >> contrasenia;

        escribirRegistro("Inicio de sesion _ usuario: " + usuario);

        string respuesta;
        bool autenticado = false;
        bool bloqueado = false;

        for (Credencial& credencial : credenciales) {
            if (credencial.usuario == usuario) {
                if (credencial.intentos >= 3) {
                    bloqueado = true;
                    break;
                }
                if (credencial.contrasenia == contrasenia) {
                    autenticado = true;
                    rol = credencial.rol;
                    break;
                } else {
                    credencial.intentos++;
                    if (credencial.intentos >= 3) {
                        bloqueado = true;
                    }
                    // Actualiza el archivo credenciales.txt con los nuevos intentos
                    ofstream archivoCredenciales("credenciales.txt");
                    if (archivoCredenciales.is_open()) {
                        for (const Credencial& c : credenciales) {
                            archivoCredenciales << c.usuario << "|" << c.contrasenia << "|" << c.rol << "|" << c.intentos << "\n";
                        }
                        archivoCredenciales.flush();
                    }

                }
            }
        }
        if (bloqueado) {
            respuesta = "Se ha superado la cantidad maxima de intentos, la cuenta ha sido bloqueada.";
            send(clienteSocket, respuesta.c_str(), respuesta.size(), 0);
        }

        if(autenticado){
            respuesta = "Acceso concedido|" + rol;
            send(clienteSocket, respuesta.c_str(), respuesta.size(), 0);
        }else {
            respuesta = "Datos de usuario incorrectos.";
            send(clienteSocket, respuesta.c_str(), respuesta.size(), 0);
        }

        while (autenticado) {
            int opcion;
            int bytesRecibidosOpcion = recv(clienteSocket, reinterpret_cast<char*>(&opcion), sizeof(opcion), 0);
            if (bytesRecibidosOpcion <= 0) {
                cerr << "Error al recibir la opción del cliente" << endl;
                break; // Salir del bucle si hay un error
            }

            switch (opcion) {
                case 1:
                    if (rol == "CONSULTA") {
                        char bufferPalabra[1024];
                        int bytesRecibidosPalabra = recv(clienteSocket, bufferPalabra, sizeof(bufferPalabra), 0);
                        if (bytesRecibidosPalabra <= 0) {
                            cerr << "Error al recibir la palabra en inglés del cliente" << endl;
                        } else {
                            bufferPalabra[bytesRecibidosPalabra] = '\0';

                            string palabraEnIngles(bufferPalabra);
                            string traduccion = traducirPalabra(palabraEnIngles, traducciones);

                            if (!traduccion.empty()) {
                                respuesta = palabraEnIngles + " en ingles es " + traduccion +" en espanol.";
                            } else {
                                respuesta = "No fue posible encontrar la traduccion para: " + palabraEnIngles;
                            }
                            send(clienteSocket, respuesta.c_str(), respuesta.size(), 0);
                        }

                    }
                    break;
                case 2:
                    if (rol == "ADMIN") {
                        char bufferTraduccion[1024];
                        int bytesRecibidosTraduccion = recv(clienteSocket, bufferTraduccion, sizeof(bufferTraduccion), 0);
                        if (bytesRecibidosTraduccion <= 0) {
                            cerr << "Error al recibir la traducción del cliente" << endl;
                        } else {
                            bufferTraduccion[bytesRecibidosTraduccion] = '\0';
                            string traduccion(bufferTraduccion);
                            string respuesta;

                            if(traduccionExiste(traducciones,traduccion)){
                                respuesta = "La traduccion ya existe.";
                            }
                            else if(agregarNuevaTraduccion(traduccion, traducciones)) {

                                respuesta = "Traduccion insertada exitosamente.";

                            } else {
                                respuesta = "No fue posible insertar la traduccion. El formato de insercion debe ser palabraEnIngles:traduccionEnEspanol.";
                            }




                            send(clienteSocket, respuesta.c_str(), respuesta.size(), 0);


                        }
                        archivoTraducciones.close();

                    }
                    break;
                case 3:
                    if(rol == "ADMIN"){
                        bool regresarAlMenu = false;
                        while(!regresarAlMenu){
                        int opcionMenuDos = 0;
                        recv(clienteSocket, reinterpret_cast<char*>(&opcionMenuDos), sizeof(opcionMenuDos), 0);
                        if(opcionMenuDos==1){
                            char bufferNuevoUsuario[1024];
                            int bytesRecibidosNuevoUsuario = recv(clienteSocket, bufferNuevoUsuario, sizeof(bufferNuevoUsuario), 0);

                            if (bytesRecibidosNuevoUsuario <= 0) {
                                cerr << "Error al recibir los datos del nuevo usuario" << endl;
                            } else {
                                bufferNuevoUsuario[bytesRecibidosNuevoUsuario] = '\0';
                                string datosNuevoUsuario(bufferNuevoUsuario);
                                size_t separadorPos = datosNuevoUsuario.find("|");
                                if (separadorPos != string::npos) {
                                    string nuevoUsuario = datosNuevoUsuario.substr(0, separadorPos);
                                    string nuevaContrasenia = datosNuevoUsuario.substr(separadorPos + 1);

                                    string respuesta;
                                    if(verificarUsuarioYContrasenia(nuevoUsuario,nuevaContrasenia)){
                                        respuesta = "Error: el usuario o la contrasenia estan en blanco.";
                                    }else{
                                        if(usuarioExiste(credenciales,nuevoUsuario)){
                                            respuesta = "Error al dar de alta el nuevo usuario: usuario existente.";
                                        }
                                        else{
                                            darDeAlta(credenciales, nuevoUsuario, nuevaContrasenia);

                                            // Actualizar el archivo credenciales.txt con los nuevos intentos
                                            ofstream archivoCredenciales("credenciales.txt");
                                            if (archivoCredenciales.is_open()) {
                                                for (const Credencial& c : credenciales) {
                                                    archivoCredenciales << c.usuario << "|" << c.contrasenia << "|" << c.rol << "|" << c.intentos << "\n";
                                                }
                                                archivoCredenciales.flush();
                                                archivoCredenciales.close();
                                            }


                                            respuesta = nuevoUsuario + " dado de alta exitosamente";
                                        }
                                    }
                                    send(clienteSocket, respuesta.c_str(), respuesta.size(), 0);


                                }
                            }

                        }else if(opcionMenuDos==2){
                            archivoCredenciales.open("credenciales.txt", ios::in | ios::out );
                            vector<string> usuariosBloqueados = listarUsuariosBloqueados(credenciales);
                            if (usuariosBloqueados.empty()) {
                                string respuesta = "No se encontraron usuarios bloqueados";
                                send(clienteSocket, respuesta.c_str(), respuesta.size(), 0);
                            } else {
                                // Enviar la lista de usuarios bloqueados al cliente
                                string listaUsuarios;
                                for (const string& usuario : usuariosBloqueados) {
                                    listaUsuarios += usuario + "\n";
                                }
                                send(clienteSocket, listaUsuarios.c_str(), listaUsuarios.size(), 0);


                                char bufferUsuarioDesbloquear[1024];
                                int bytesRecibidosUsuarioDesbloquear = recv(clienteSocket, bufferUsuarioDesbloquear, sizeof(bufferUsuarioDesbloquear), 0);
                                if (bytesRecibidosUsuarioDesbloquear <= 0) {
                                    cerr << "Error al recibir la seleccion de usuario a desbloquear" << endl;
                                } else {
                                    bufferUsuarioDesbloquear[bytesRecibidosUsuarioDesbloquear] = '\0';
                                    string usuarioDesbloquear(bufferUsuarioDesbloquear);


                                    bool usuarioBloqueado = false;
                                    for (const string& usuarioBloq : usuariosBloqueados) {
                                        if (usuarioBloq == usuarioDesbloquear) {
                                            usuarioBloqueado = true;
                                            break;
                                        }
                                    }
                                    if(usuarioBloqueado){
                                        for (Credencial& credencial : credenciales) {
                                            if (credencial.usuario == usuarioDesbloquear) {
                                                credencial.intentos = 0;
                                                string respuesta = usuarioDesbloquear + " desbloqueado correctamente";
                                                send(clienteSocket, respuesta.c_str(), respuesta.size(), 0);
                                                break;
                                            }
                                        }
                                    }else {

                                        string respuesta = usuarioDesbloquear + " no figura como un usuario bloqueado";
                                        send(clienteSocket, respuesta.c_str(), respuesta.size(), 0);
                                    }
                                    // Actualizar el archivo credenciales.txt con los nuevos intentos
                                    ofstream archivoCredenciales("credenciales.txt");
                                    if (archivoCredenciales.is_open()) {
                                        for (const Credencial& c : credenciales) {
                                            archivoCredenciales << c.usuario << "|" << c.contrasenia << "|" << c.rol << "|" << c.intentos << "\n";
                                        }
                                        archivoCredenciales.flush();
                                    }
                                    archivoCredenciales.close();
                                }
                            }
                        }else if(opcionMenuDos==3){
                            break;
                        }
                        }
                    }
                    break;

                case 4:
                    if (rol == "ADMIN") {

                        ifstream archivoRegistro("server.log", ios::in);

                        string linea;
                        while (getline(archivoRegistro, linea)) {

                            send(clienteSocket, (linea + "\n").c_str(), linea.size() + 1, 0);

                        }


                        string finRegistro = "FIN_REGISTRO";
                        send(clienteSocket, finRegistro.c_str(), finRegistro.size(), 0);


                        archivoRegistro.close();
                        break;


                    }
                    break;
                case 5:
                    escribirRegistro("Cierre de sesion _ usuario: " + usuario);
                    autenticado = false;
                    break;
                default:
                    cout << "Opcion no valida. Intente de nuevo." << endl;
                    break;
            }

        }



    }

    // Cerrar socket del servidor y limpiar Winsock
    closesocket(servidorSocket);
    WSACleanup();

    return 0;
}












