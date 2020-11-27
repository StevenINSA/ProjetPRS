all:serveur1-No11

#-Wall : active tous les warnings
#$@ = cible = arbre.o
#$< = première dépendance = arbre.c
#-c compiler sans édition de liens (sans faire un exécutable puisqu'on n'a pas de main)
serveur1-No11: serveur1-No11.o
	gcc -Wall $^ -o $@

serveur1-No11.o: serveur1-No11.c
	gcc -Wall -c $< -o $@

clean:
	rm -f all *.o
