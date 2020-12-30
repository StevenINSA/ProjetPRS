all:serveur1-No11 serveur3-No11 serveur2-No11 serveur3bis-No11 serveur3 test serveur1bis

#-Wall : active tous les warnings
#$@ = cible = arbre.o
#$< = première dépendance = arbre.c
#-c compiler sans édition de liens (sans faire un exécutable puisqu'on n'a pas de main)

serveur1bis: serveur1bis.o
	gcc -Wall $^ -o $@

serveur1bis.o: serveur1bis.c
	gcc -Wall -c $< -o $@

test: test.o
	gcc -Wall $^ -o $@

test.o: test.c
	gcc -Wall -c $< -o $@

serveur3-No11: serveur3-No11.o
	gcc -Wall $^ -o $@

serveur3-No11.o: serveur3-No11.c
	gcc -Wall -c $< -o $@

serveur3: serveur3.o
		gcc -Wall $^ -o $@

serveur3.o: serveur3.c
		gcc -Wall -c $< -o $@


serveur3bis-No11: serveur3bis-No11.o
		gcc -Wall $^ -o $@

serveur3bis-No11.o: serveur3bis-No11.c
		gcc -Wall -c $< -o $@


serveur1-No11: serveur1-No11.o
	gcc -Wall $^ -o $@

serveur1-No11.o: serveur1-No11.c
	gcc -Wall -c $< -o $@

serveur2-No11: serveur2-No11.o
	gcc -Wall $^ -o $@

serveur2-No11.o: serveur2-No11.c
	gcc -Wall -c $< -o $@


clean:
	rm -f all *.o
