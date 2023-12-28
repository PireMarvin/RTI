.SILENT:
APPLIC= Client Serveur #ClientTest ServeurTest
TCP_SOURCE=../
COMP2=g++ -Wno-unused-parameter 
COMP=-c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../UNIX_DOSSIER_FINAL -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++
LIB=/usr/lib64/libQt5Widgets.so /usr/lib64/libQt5Gui.so /usr/lib64/libQt5Core.so /usr/lib64/libGL.so -lpthread
MYSQL=-I/usr/include/mysql -m64 -L/usr/lib64/mysql -lmysqlclient

all:	$(APPLIC)

Client:				mainclient.o windowclient.o moc_windowclient.o TCP.o
					$(COMP2) -o Client mainclient.o windowclient.o moc_windowclient.o TCP.o -I../ $(LIB)
					echo "	Creation de Client"

Serveur:			Serveur.cpp TCP.o OVESP.o SGBD.o
					$(COMP2) -o Serveur Serveur.cpp TCP.o OVESP.o SGBD.o -I../ $(LIB) $(MYSQL)
					echo "	Creation de Serveur"

mainclient.o:		mainclient.cpp
					$(COMP2) $(COMP) -o mainclient.o mainclient.cpp 
					echo "	Compilation de mainclient.o"

windowclient.o:		windowclient.cpp windowclient.h
					$(COMP2) $(COMP) -o windowclient.o windowclient.cpp -I../
					echo "	Compilation de windowclient.o"

moc_windowclient.o:	moc_windowclient.cpp ui_windowclient.h
					$(COMP2) $(COMP) -o moc_windowclient.o moc_windowclient.cpp
					echo "	Compilation de moc_windowclient.o"

TCP.o:				$(TCP_SOURCE)TCP.cpp $(TCP_SOURCE)TCP.h
					g++ -o TCP.o -c $(TCP_SOURCE)TCP.cpp
					echo "	Compilation de TCP.o"

OVESP.o:			OVESP.cpp OVESP.h SGBD.o
					$(COMP2) $(COMP) -o OVESP.o OVESP.cpp $(MYSQL)
					echo "	Compilation de OVESP.o"

SGBD.o:				SGBD.c SGBD.h
					$(COMP2) $(COMP) -o SGBD.o SGBD.c $(MYSQL)
					echo "	Compilation de SGBD.o"


clean:
			rm -f *.o
			echo "	Suppression des >> .O <<"

clobber:	clean
			rm -f $(APPLIC)
			echo "	Suppression de  >> $(APPLIC) <<"