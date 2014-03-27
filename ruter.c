#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include "helpers.h"

int out, in, nod_id, seq_no = 0 ;
int timp = -1;
int gata = FALSE;

msg *mesaje_noi ;
int start_noi , end_noi ;
msg *mesaje_vechi ;
int start_vechi , end_vechi ;

msg LSADatabase[KIDS];
int topologie[ KIDS ][ KIDS ] ;

void interschimbare() { 
	int i = start_noi ;
	start_vechi = start_noi ;
	end_vechi = end_noi ;
	while ( i != end_noi ) { 
		mesaje_vechi[i].type = mesaje_noi[i].type ; 
		mesaje_vechi[i].creator = mesaje_noi[i].creator ;
		mesaje_vechi[i].seq_no = mesaje_noi[i].seq_no ;
		mesaje_vechi[i].sender = mesaje_noi[i].sender ;
		mesaje_vechi[i].next_hop = mesaje_noi[i].next_hop ;
		mesaje_vechi[i].time = mesaje_noi[i].time ;
		mesaje_vechi[i].add = mesaje_noi[i].add ;
		mesaje_vechi[i].len = mesaje_noi[i].len ;
		memcpy( mesaje_vechi[i].payload, mesaje_noi[i].payload, sizeof(mesaje_noi[i].payload));
		i ++ ;
		if ( i==1400 ) i = 0 ;
	}
}

void verificare_mesaj(msg mesaj){ 
	if ( LSADatabase[mesaj.creator].type == - 1 ) {
		LSADatabase[mesaj.creator].type = mesaj.type ;
		LSADatabase[mesaj.creator].creator = mesaj.creator ;
		LSADatabase[mesaj.creator].seq_no = mesaj.seq_no ;
		LSADatabase[mesaj.creator].sender = mesaj.sender ;
		LSADatabase[mesaj.creator].next_hop = mesaj.next_hop ;
		LSADatabase[mesaj.creator].time = mesaj.time ;
		LSADatabase[mesaj.creator].add = mesaj.add ;
		LSADatabase[mesaj.creator].len = mesaj.len ;
		memcpy( LSADatabase[mesaj.creator].payload, mesaj.payload, sizeof(mesaj.payload));
	} else {  
		if (LSADatabase[mesaj.creator].time < mesaj.time){
			LSADatabase[mesaj.creator].type = mesaj.type ;
			LSADatabase[mesaj.creator].creator = mesaj.creator ;
			LSADatabase[mesaj.creator].seq_no = mesaj.seq_no ;
			LSADatabase[mesaj.creator].sender = mesaj.sender ;
			LSADatabase[mesaj.creator].next_hop = mesaj.next_hop ;
			LSADatabase[mesaj.creator].time = mesaj.time ;
			LSADatabase[mesaj.creator].add = mesaj.add ;
			LSADatabase[mesaj.creator].len = mesaj.len ;
			memcpy( LSADatabase[mesaj.creator].payload, mesaj.payload, sizeof(mesaj.payload));
		}
	} 
	int i = start_noi ;
	while ( i != end_noi ) { printf("da\n");
		if (mesaje_noi[i].creator==mesaj.creator && mesaje_noi[i].seq_no==mesaj.seq_no){ //TODO!!!!!!
			if (mesaje_noi[i].time<=mesaj.time) {
				mesaje_noi[i].type = mesaj.type ;
				mesaje_noi[i].creator = mesaj.creator ;
				mesaje_noi[i].seq_no = mesaj.seq_no ;
				mesaje_noi[i].sender = mesaj.sender ;
				mesaje_noi[i].next_hop = mesaj.next_hop ;
				mesaje_noi[i].time = mesaj.time ;
				mesaje_noi[i].add = mesaj.add ;
				mesaje_noi[i].len = mesaj.len ;
				memcpy( mesaje_noi[i].payload, mesaj.payload, sizeof(mesaj.payload));
				break; 
			}
			break ;
		}
		i ++ ; 
		if (i==1400) i=0;
	}	
	if ( i == end_noi ) {
		mesaje_noi[i].type = mesaj.type ;
		mesaje_noi[i].creator = mesaj.creator ;
		mesaje_noi[i].seq_no = mesaj.seq_no ;
		mesaje_noi[i].sender = mesaj.sender ;
		mesaje_noi[i].next_hop = mesaj.next_hop ;
		mesaje_noi[i].time = mesaj.time ;
		mesaje_noi[i].add = mesaj.add ;
		mesaje_noi[i].len = mesaj.len ;
		memcpy( mesaje_noi[i].payload, mesaj.payload, sizeof(mesaj.payload));
		end_noi++;
		if ( end_noi==1400 ) end_noi = 0;
	} //printf("%d %d\n",start_noi,end_noi);
}

//nu modificati numele, modalitatea de alocare si initializare a tabelei de rutare 
//se foloseste la mesajele de tip 8/10 (deja implementate aici) si la logare (implementata in simulator.c)
int tab_rutare [KIDS][2];

void calcul_tabela_rutare() { //printf("da1\n");
	int i,j,k;
	int indici[5][5];
	int copie[5][5];
	for (i=0;i<5;i++)
		for (j=0;j<5;j++)
			copie[i][j]=topologie[i][j];

	for (i=0;i<5;i++)
		for (j=0;j<5;j++)
			if ((i==j) || (topologie[i][j]==100))
				indici[i][j]=0;
			else indici[i][j]= i;

	for (k=0;k<5;k++)
		for (i=0;i<5;i++)
			for (j=0;j<5;j++)
				if (copie[i][j]>copie[i][k]+copie[k][j]) {
					copie[i][j]=copie[i][k]+copie[k][j];
					indici[i][j]=indici[k][j];
				}
	int hop;
	for (i=0;i<KIDS;i++)
		if (copie[nod_id][i]!=0){
			tab_rutare[i][0]=copie[nod_id][i];

			hop = i;
			while (indici[nod_id][hop]!=nod_id){
				hop=indici[nod_id][hop];
			}
			tab_rutare[i][1]=hop;
		} 
}

void procesare_eveniment(msg mevent) {

	char string[1400];
	msg mesaj_reply ;
	int i , j , k , vecin , cost , nr_vec = 0 ;

	if (mevent.add == TRUE) {
		printf ("Nod %d, msg tip eveniment - am aderat la topologie la pasul %d\n", nod_id, timp); 
	}
	else
		printf ("Timp %d, Nod %d, procesare eveniment\n", timp, nod_id);

	//aveti de implementat tratarea evenimentelor si trimiterea mesajelor ce tin de protocolul de rutare
	//in campul payload al mesajului de tip 7 e linia de fisier corespunzatoare respectivului eveniment
	//(optional) vezi simulator.c, liniile 93-119 (trimitere mes tip 7) si liniile 209-219 (parsare fisier evenimente) 
					 
	//rutere direct implicate in evenimente, care vor primi mesaje de tip 7 de la simulatorul central:
	//eveniment tip 1: ruterul nou adaugat la retea  (ev.d1  - vezi liniile indicate)
	//eveniment tip 2: capetele noului link (ev.d1 si ev.d2)
	//eveniment tip 3: capetele linkului suprimat (ev.d1 si ev.d2)
	//evenimet tip 4:  ruterul sursa al pachetului (ev.d1)

	switch (mevent.type){
		case 1 : 
			mesaj_reply.type = 2 ;
			mesaj_reply.time = timp ;
			mesaj_reply.sender = nod_id ;
			mesaj_reply.creator = nod_id ;
			mesaj_reply.seq_no = seq_no ++ ;

			memcpy(string, mevent.payload, sizeof(mevent.payload));
			
			i = 0 ; 
			while (string[i]!=' ')
				i++;
			i++;
			k=0;
			while (string[i]!=' '){
				k=10*k+string[i]-'0';
				i++;
			} 

			i ++ ;
			while (string[i]!=' '){
				nr_vec=10*nr_vec+string[i]-'0';
				i++;
			} 
			for ( j=0; j<nr_vec; j++ ){
				i++;
				vecin = 0 ;
				while ((string[i]!=' ') && (string[i]!='\0') && (string[i]!='\n')){
					vecin=10*vecin+string[i]-'0';
					i++;
				}

				i++;
				cost = 0 ;
				while ((string[i]!=' ') && (string[i]!='\0') && (string[i]!='\n')){
					cost=10*cost+string[i]-'0';
					i++;
				}	//printf("%d %d %d %d\n",k,nr_vec,vecin,cost);
				memcpy(mesaj_reply.payload,&k,sizeof(int));
				memcpy(mesaj_reply.payload+sizeof(int),&vecin,sizeof(int));
				memcpy(mesaj_reply.payload+2*sizeof(vecin),&cost,sizeof(int));
				mesaj_reply.len=3*sizeof(int); 
				write(out, &mesaj_reply, sizeof(msg));
			}	
		break;

		case 2 :
			timp = mevent.time ;

			mesaj_reply.type = 2 ;
			mesaj_reply.sender = nod_id ;
			mesaj_reply.creator = nod_id ;
			mesaj_reply.seq_no = seq_no ++ ;
			mesaj_reply.time = timp ;

			memcpy(string, mevent.payload, sizeof(mevent.payload));

			i = 0 ; 
			while (string[i]!=' ')
				i++;

			i++;
			k=0;
			while (string[i]!=' '){
				k=10*k+string[i]-'0';
				i++;
			} 
			
			i ++ ;
			while (string[i]!=' '){
				vecin=10*vecin+string[i]-'0';
				i++;
			} 
			
			i++;
			cost = 0 ;
			while ((string[i]!=' ') && (string[i]!='\0') && (string[i]!='\n')){
				cost=10*cost+string[i]-'0';
				i++;
			}	//printf("%d %d %d\n",k,vecin,cost);
			mesaj_reply.len=3*sizeof(int);
			memcpy(mesaj_reply.payload,&k,sizeof(int));
			memcpy(mesaj_reply.payload+sizeof(int),&vecin,sizeof(int));
			memcpy(mesaj_reply.payload+2*sizeof(vecin),&cost,sizeof(int));
			write(out, &mesaj_reply, sizeof(msg));
		break ;

		case 3 :
			timp = mevent.time ;
			memcpy(string, mevent.payload, sizeof(mevent.payload));

			i = 0 ; 
			while (string[i]!=' ')
				i++;

			i++;
			k=0;
			while (string[i]!=' '){
				k=10*k+string[i]-'0';
				i++;
			} 
			
			i ++ ;
			while ((string[i]!=' ') && (string[i]!='\0') && (string[i]!='\n')){
				vecin=10*vecin+string[i]-'0';
				i++;
			} 
			memcpy(mesaj_reply.payload,&k,sizeof(int));
			memcpy(mesaj_reply.payload+sizeof(int),&vecin,sizeof(int));
			cost = DRUMAX;
			memcpy(mesaj_reply.payload+2*sizeof(int),&cost,sizeof(int));

			topologie[nod_id][k] = DRUMAX;
			topologie[k][nod_id] = DRUMAX;
			topologie[nod_id][vecin] = DRUMAX;
			topologie[vecin][nod_id] = DRUMAX;

			write(out, &mesaj_reply, sizeof(msg));
		break ;

		case 4 :
			i = 0 ; 
			while (string[i]!=' ')
				i++;

			i++;
			k=0;
			while (string[i]!=' '){
				k=10*k+string[i]-'0';
				i++;
			} 
			
			i ++ ;
			while ((string[i]!=' ') && (string[i]!='\0') && (string[i]!='\n')){
				vecin=10*vecin+string[i]-'0';
				i++;
			} 
			mesaj_reply.type = 4 ;
			mesaj_reply.sender = nod_id ;
			mesaj_reply.creator = nod_id ;
			mesaj_reply.seq_no = seq_no ++ ;
			mesaj_reply.time = timp ;
			mesaj_reply.time = tab_rutare[vecin][1];
			memcpy(mesaj_reply.payload,&vecin,sizeof(int));

			write(out, &mesaj_reply, sizeof(msg));
		break ;

	}
}

int main (int argc, char ** argv)
{
	msg mesaj, mesaj_event;
	int cit , k , i , j ;
	int event = FALSE;

	mesaje_noi = ( msg *)  malloc ( 1400 * sizeof( msg ) ) ;
	start_noi = 0 , end_noi = 0 ;
	mesaje_vechi = ( msg * ) malloc ( 1400 * sizeof( msg ) ) ;
	start_vechi = 0 , end_vechi = 0 ;


	out = atoi(argv[1]);  //legatura pe care se trimit mesaje catre simulatorul central (toate mesajele se trimit pe aici)
	in = atoi(argv[2]);   //legatura de pe care se citesc mesajele

	nod_id = atoi(argv[3]); //procesul curent participa la simulare numai dupa ce nodul cu id-ul lui este adaugat in topologie 
				
	//tab_rutare[k][0] reprezinta costul drumului minim de la ruterul curent (nod_id) la ruterul k 
	//tab_rutare[k][1] reprezinta next_hop pe drumul minim de la ruterul curent (nod_id) la ruterul k 							
	for (k = 0; k < KIDS; k++) {
		tab_rutare[k][0] = DRUMAX;  // drum =DRUMAX daca ruterul k nu e in retea sau informatiile despre el nu au ajuns la ruterul curent
		tab_rutare[k][1] = -1; //in cadrul protocolului(pe care il veti implementa), next_hop =-1 inseamna ca ruterul k nu e (inca) cunoscut de ruterul nod_id (vezi mai sus) 
	}
	
	for ( i = 0 ; i < KIDS ; i ++ ) LSADatabase[ i ].type = -1 ;
	
	for ( i = 0 ; i < KIDS ; i ++ )
		for ( j = 0 ; j < KIDS ; j ++ )
			topologie[i][j] = DRUMAX ;

	//printf ("Nod %d, pid %u alive & kicking\n", nod_id, getpid());

	if (nod_id == 0) { //sunt deja in topologie
		timp = -1; //la momentul 0 are loc primul eveniment
		mesaj.type = 5; //finish procesare mesaje timp -1
		mesaj.sender = nod_id;
		write (out, &mesaj, sizeof(msg)); 
		//printf ("TRIMIS Timp %d, Nod %d, msg tip 5 - terminare procesare mesaje vechi din coada\n", timp, nod_id);
	
	}

	int nod1,nod2,cost;
	msg mesaj_reply ;

	while (!gata) {
		cit = read(in, &mesaj, sizeof(msg));
		
		if (cit <= 0) {
			printf ("Adio, lume cruda. Timp %d, Nod %d, msg tip %d cit %d\n", timp, nod_id, mesaj.type, cit);
			exit (-1);
		}

		//printf("%d %s\n",mesaj.type,mesaj.payload);
		switch (mesaj.type) {
			
			//1,2,3,4 sunt mesaje din protocolul link state; 
			//actiunea imediata corecta la primirea unui pachet de tip 1,2,3,4 este buffer-area 
			//(punerea in coada /coada new daca sunt 2 cozi - vezi enunt)
			 
			case 1 :  
					//printf ("Timp %d, Nod %d, msg tip 1 - LSA\n", timp, nod_id);
					verificare_mesaj(mesaj);

				break;
				
			case 2 : 
					//printf ("Timp %d, Nod %d, msg tip 2 - Database Request\n", timp, nod_id);
					verificare_mesaj(mesaj);

				break;
				
			case 3 : 
					//printf ("Timp %d, Nod %d, msg tip 3 - Database Reply\n", timp, nod_id);
					verificare_mesaj(mesaj);
				
				break;
				
			case 4 : 
					//printf ("Timp %d, Nod %d, msg tip 4 - pachet de date (de rutat)\n", timp, nod_id);
					verificare_mesaj(mesaj);
				
				break; 
			
			case 6 :
				
				printf ("Timp %d, Nod %d, msg tip 6 - incepe procesarea mesajelor puse din coada la timpul anterior (%d)\n", timp, nod_id, timp-1);
				
				int mai_sunt_mesaje_vechi = TRUE; 
				timp++;
				
				interschimbare() ;
				start_noi = end_noi = 0 ;
				msg mesaj_vechi ;

				if ( start_vechi == end_vechi ) mai_sunt_mesaje_vechi = FALSE;
				
				while (mai_sunt_mesaje_vechi) { //printf("%d\n",mesaje_vechi[start_vechi].type);
					//	procesez toate mesajele venite la timpul anterior
					//	(sau toate mesajele primite inainte de inceperea timpului curent - marcata de mesaj de tip 6) modif
					//	la acest pas/timp NU se vor procesa mesaje venite DUPA inceperea timpului curent
					//cand trimiteti mesaje de tip 4 nu uitati sa setati (inclusiv) campurile, necesare pt logare:  
					//mesaj.time, mesaj.creator,mesaj.seq_no, mesaj.sender, mesaj.next_hop
					//la tip 4 - creator este sursa initiala a pachetului rutat
					
					//cand coada devine goala sau urmatorul mesaj are timpul de trimitere == pasul curent de timp:
					
					mesaj_vechi.type = mesaje_vechi[start_vechi].type ;
					mesaj_vechi.creator = mesaje_vechi[start_vechi].creator ;
					mesaj_vechi.seq_no = mesaje_vechi[start_vechi].seq_no ;
					mesaj_vechi.sender = mesaje_vechi[start_vechi].sender ;
					mesaj_vechi.next_hop = mesaje_vechi[start_vechi].next_hop ;
					mesaj_vechi.time = mesaje_vechi[start_vechi].time ;
					mesaj_vechi.add = mesaje_vechi[start_vechi].add ;
					mesaj_vechi.len = mesaje_vechi[start_vechi].len ;
					memcpy( mesaj_vechi.payload, mesaje_vechi[start_vechi].payload, sizeof(mesaje_vechi[start_vechi].payload));

					if (mesaj_vechi.time < timp) { //printf("%d\n",mesaj_vechi.type);
						switch (mesaj_vechi.type) {
							case 1 :
							break ;
							case 2 :
								memcpy(&nod1,mesaj_vechi.payload,sizeof(int));
								memcpy(&nod2,mesaj_vechi.payload+sizeof(int),sizeof(int));
								memcpy(&cost,mesaj_vechi.payload+2*sizeof(int),sizeof(int));
								topologie[nod1][nod2]=cost;
								topologie[nod2][nod1]=cost;
								mesaj_reply.type = 3 ;
								for (i=0;i<KIDS;i++){
									if (LSADatabase[i].type!=-1)
										write(out, &LSADatabase[i], sizeof(msg));
								}
							break ;
							case 3 :
							//printf("Timp %d Procesez msg type 1-3 creator %d \n", timp, mesaj_vechi.creator);
							//printf("%d\n",mesaj_vechi.type);
							//printf("%s\n",mesaj_vechi.payload);
							break ;
							case 4 :
							break ;
						}
					}
						
					start_vechi ++ ;
					if ( start_vechi == 1400 ) start_vechi = 0 ;
					if ( start_vechi == end_vechi ) mai_sunt_mesaje_vechi = FALSE;
				}
				
				//procesez mesaj eveniment
				if (event) { 
					mesaj_event.time = timp ;
					i = 0 ;
					mesaj_event.type=0;
					while (mesaj.payload[i]!=' ' && mesaj.payload[i]!='\n' && mesaj.payload[i]!='\0'){
						mesaj_event.type=10*mesaj_event.type+mesaj.payload[i]-'0';
						i++;
					} //printf("%d\n",mesaj_event.type);
					memcpy(mesaj_event.payload,mesaj.payload,sizeof(mesaj.payload)); 
					procesare_eveniment(mesaj_event);
					event = FALSE;
				}

				//calculez tabela de rutare
				//calcul_tabela_rutare();
				
				//nu mai sunt mesaje vechi, am procesat evenimentul(daca exista), am calculat tabela de rutare(in aceasta ordine)
				//trimit mesaj de tip 5 (terminare pas de timp)

				mesaj.type = 5; 
				mesaj.sender = nod_id;
				//printf("da\n");
				//write (out, &mesaj, sizeof(msg)); 
				
			break;
			
			case 7 : //complet implementat - nu modificati! (exceptie afisari on/off)
					//aici se trateaza numai salvarea mesajului de tip eveniment(acest mesaj nu se pune in coada), pentru a fi procesat la finalul acestui pas de timp
					//procesarea o veti implementa in functia procesare_eveniment(), apelata in case 6
				
				event = TRUE;
				memcpy (&mesaj_event, &mesaj, sizeof(msg));
				
				if (mesaj.add == TRUE) 
					timp = mesaj.time+1; //initializam timpul pentru un ruter nou 
				
				break;
			
			case 8 : //complet implementat - nu modificati! (exceptie afisari on/off)
					//aici doar se apeleaza calcul_tabela_rutare() trebuie sa completati voi implementarea in corpul functiei
				
				//printf ("Timp %d, Nod %d, msg tip 8 - cerere tabela de rutare\n", timp, nod_id);
				
				mesaj.type = 10;  //trimitere tabela de rutare
				mesaj.sender = nod_id;
				mesaj.time = timp;
				memcpy (mesaj.payload, &tab_rutare, sizeof (tab_rutare));
				//Observati ca acest tip de mesaj (8) se proceseaza imediat - nu se pune in nicio coada (vezi enunt)
				write (out, &mesaj, sizeof(msg)); 
				timp++;
				
				break;
				
			case 9 : //complet implementat - nu modificati! (exceptie afisari on/off)
				
				//Aici poate sa apara timp -1 la unele "noduri"
				//E ok, e vorba de procesele care nu reprezentau rutere in retea, deci nu au de unde sa ia valoarea corecta de timp
				//Alternativa ar fi fost ca procesele neparticipante la simularea propriu-zisa sa ramana blocate intr-un apel de read()
				//printf ("Timp %d, Nod %d, msg tip 9 - terminare simulare\n", timp, nod_id);
				gata = TRUE;
				
				break;
				

			default:
				printf ("\nEROARE: Timp %d, Nod %d, msg tip %d - NU PROCESEZ ACEST TIP DE MESAJ\n", timp, nod_id, mesaj.type);
				exit (-1);
		}			
	}

return 0;

}
