#include "./presentation/presentation.h"

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *fctThreadFenetreGraphique(void *);
void *fctThreadEvenements(void *);
void *fctThreadStanley(void *);
void *fctThreadEnnemis(void *);
void *fctThreadGuepe(void *);
void *fctThreadChenilleG(void *);
void *fctThreadChenilleD(void *);
void *fctThreadAraigneeG(void *);
void *fctThreadAraigneeD(void *);
void *fctThreadInsecticideG(void *);
void *fctThreadInsecticideD(void *);

void handlerSIGINT(int);
void handlerSIGALRM(int);
void handlerSIGUSR1(int);
void handlerSIGUSR2(int);
void handlerSIGQUIT(int); 

void destructeurVS(void *p);

pthread_t threadFenetreGraphique, threadEvenement, threadStanley, threadEnnemis, threadEnnemi, threadInsecticide;

pthread_cond_t condEvenement;
pthread_cond_t condEchec;

pthread_key_t keySpec;

pthread_mutex_t mutexEtatJeu;
pthread_mutex_t mutexEvenement;
pthread_mutex_t mutexEchec;

#define DELAI_MIN 1100000
#define DELAI_MAX 1600000

typedef struct{
    int presence;
    pthread_t tid;
} S_PRESENCE;

typedef struct{
    int position;
    int orientation;
} S_LOCALISATION;


typedef struct{
    int etatStanley;
    int positionStanley;
    int actionStanley;
    int etatAmis[5];
    S_PRESENCE guepes[2];
    S_PRESENCE chenillesG[5];
    S_PRESENCE chenillesD[7];
    S_PRESENCE araigneesG[5];
    S_PRESENCE araigneesD[5];
    S_PRESENCE insecticidesG[4];
    S_PRESENCE insecticidesD[4];
    int score;
    int nbEchecs;
} S_ETAT_JEU;

S_ETAT_JEU etatJeu = 
     { BAS, 1, NORMAL,
       { NORMAL, NORMAL, NORMAL, NORMAL, NORMAL },
       { { AUCUN, 0 }, { AUCUN, 0 } },
       { { AUCUN, 0 }, { AUCUN, 0 }, { AUCUN, 0 }, { AUCUN, 0 }, { AUCUN, 0 } },
       { { AUCUN, 0 }, { AUCUN, 0 }, { AUCUN, 0 }, { AUCUN, 0 }, { AUCUN, 0 }, 
         { AUCUN, 0 }, { AUCUN, 0 } },
       { { AUCUN, 0 }, { AUCUN, 0 }, { AUCUN, 0 }, { AUCUN, 0 }, { AUCUN, 0 } },
       { { AUCUN, 0 }, { AUCUN, 0 }, { AUCUN, 0 }, { AUCUN, 0 }, { AUCUN, 0 } },
       { { AUCUN, 0 }, { AUCUN, 0 }, { AUCUN, 0 }, { AUCUN, 0 } },
       { { AUCUN, 0 }, { AUCUN, 0 }, { AUCUN, 0 }, { AUCUN, 0 } },
       0, 0 };

int evenement = AUCUN; 
int echec = AUCUN;

void *fctThreadFenetreGraphique(void *param){
    int i;

    while(true){
        pthread_mutex_lock(&mutexEtatJeu);

        restaurerImageInterne();

        afficherStanley(etatJeu.etatStanley, etatJeu.positionStanley, etatJeu.actionStanley);

        afficherAmi(FLEUR_HG, etatJeu.etatAmis[FLEUR_HG]);
        afficherAmi(FLEUR_HD, etatJeu.etatAmis[FLEUR_HD]);
        afficherAmi(FLEUR_BG, etatJeu.etatAmis[FLEUR_BG]);
        afficherAmi(FLEUR_BD, etatJeu.etatAmis[FLEUR_BD]);
        afficherAmi(CHAT, etatJeu.etatAmis[CHAT]);

        for(i = 0; i < 2; i++){
            if(etatJeu.guepes[i].presence == NORMAL){
                afficherGuepe(i);
            }
        }

        for(i = 0; i < 5; i++){
            if(etatJeu.chenillesG[i].presence == NORMAL){
                afficherChenilleG(i);
            }
        }

        for(i = 0; i < 7; i++){
            if(etatJeu.chenillesD[i].presence == NORMAL){
                afficherChenilleD(i);
            }
        }

        for(i = 0; i < 5; i++){
            if(etatJeu.araigneesG[i].presence == NORMAL){
                afficherAraigneeG(i);
            }

            if(etatJeu.araigneesD[i].presence == NORMAL){
                afficherAraigneeD(i);
            }
        }

        for(i = 0; i < 4; i++){
            if(etatJeu.insecticidesG[i].presence == NORMAL){
                afficherInsecticideG(i);
            }

            if(etatJeu.insecticidesD[i].presence == NORMAL){
                afficherInsecticideD(i + 1);
            }
        }


        pthread_mutex_unlock(&mutexEtatJeu);

        afficherScore(etatJeu.score);
        afficherEchecs(etatJeu.nbEchecs);

        actualiserFenetreGraphique();

        usleep(100000);
    }
    
    pthread_exit(0);
}

void *fctThreadEvenements(void *param){
    while(true){
        pthread_mutex_lock(&mutexEvenement);        
        evenement = lireEvenement();
        pthread_mutex_unlock(&mutexEvenement);

        switch(evenement){
            case SDL_QUIT:
                exit(0);

            case SDLK_UP:
                break;

            case SDLK_DOWN:
                break;

            case SDLK_LEFT:
                break;

            case SDLK_RIGHT:
                break;

            case SDLK_SPACE:
                break;

        }
        
        pthread_cond_signal(&condEvenement);
        usleep(100000);
    }

    pthread_exit(0);
}

void *fctThreadStanley(void *param){
    while(true){

        // attendre sur condEvenement l’événement produit par le joueur
        pthread_mutex_lock(&mutexEvenement);
        while(evenement == AUCUN){
            pthread_cond_wait(&condEvenement, &mutexEvenement);
        }
        pthread_mutex_unlock(&mutexEvenement);

        pthread_mutex_lock(&mutexEchec);

        if(echec == AUCUN){
            pthread_mutex_unlock(&mutexEchec);
            
            pthread_mutex_lock(&mutexEtatJeu);

            switch(etatJeu.etatStanley){
                case BAS:
                    switch(evenement){
                        case SDLK_SPACE:
                            if(etatJeu.positionStanley != 1){
                                etatJeu.actionStanley = SPRAY;

                                pthread_mutex_unlock(&mutexEtatJeu);
                                usleep(200000);
                                pthread_mutex_lock(&mutexEtatJeu);

                                etatJeu.actionStanley = NORMAL;

                                if(etatJeu.positionStanley == 2){
                                    if(etatJeu.guepes[0].presence == NORMAL && etatJeu.etatAmis[CHAT] != TOUCHE){
                                        etatJeu.score++;

                                        // envoi du signal SIGINT à ThreadGuepe pour tuer la guêpe
                                        pthread_kill(etatJeu.guepes[0].tid, SIGINT);
                                    }

                                    if(etatJeu.guepes[1].presence == NORMAL && etatJeu.etatAmis[CHAT] != TOUCHE){
                                        etatJeu.score++;

                                        // envoi du signal SIGINT à ThreadGuepe pour tuer la guêpe
                                        pthread_kill(etatJeu.guepes[1].tid, SIGINT);
                                    }
                                }

                                if(etatJeu.positionStanley == 0){
                                    if(etatJeu.araigneesG[4].presence == NORMAL && etatJeu.etatAmis[FLEUR_BG] != TOUCHE){
                                        etatJeu.score++;

                                        // envoi du signal SIGUSR2 à ThreadAraigneeG pour tuer l'araignée
                                        pthread_kill(etatJeu.araigneesG[4].tid, SIGUSR2);
                                    }

                                    else if(etatJeu.araigneesG[4].presence == AUCUN){
                                        // création insecticide gauche
                                        pthread_create(&threadInsecticide, NULL, fctThreadInsecticideG, NULL);
                                    }
                                }

                                if(etatJeu.positionStanley == 3){
                                    if(etatJeu.araigneesD[0].presence == NORMAL && etatJeu.etatAmis[FLEUR_BD] != TOUCHE){
                                        etatJeu.score++;

                                        // envoi du signal SIGUSR2 à ThreadAraigneeD pour tuer l'araignée
                                        pthread_kill(etatJeu.araigneesD[0].tid, SIGUSR2);
                                    }

                                    else if(etatJeu.araigneesD[0].presence == AUCUN){
                                        // création insecticide droite
                                        pthread_create(&threadInsecticide, NULL, fctThreadInsecticideD, NULL);
                                    }
                                }
                            }

                            break;
                            
                        case SDLK_LEFT:
                            if(etatJeu.positionStanley > 0){
                                etatJeu.positionStanley--;
                            }

                            break;

                        case SDLK_RIGHT:
                            if(etatJeu.positionStanley < 3){
                                etatJeu.positionStanley++;
                            }
                            
                            break;

                        case SDLK_UP:
                            if(etatJeu.positionStanley == 1){
                                etatJeu.etatStanley = ECHELLE;
                            }

                            break;
                    }

                    break;

                case HAUT:
                    switch(evenement){
                        case SDLK_SPACE:
                            if(etatJeu.positionStanley != 2){
                                etatJeu.actionStanley = SPRAY;

                                pthread_mutex_unlock(&mutexEtatJeu);
                                usleep(200000);
                                pthread_mutex_lock(&mutexEtatJeu);

                                etatJeu.actionStanley = NORMAL;

                                if(etatJeu.positionStanley == 1){
                                    if(etatJeu.chenillesG[2].presence == NORMAL && etatJeu.etatAmis[FLEUR_HG] != TOUCHE){
                                        etatJeu.score++;

                                        // envoi du signal SIGUSR1 à ThreadChenilleG pour tuer la chenille
                                        pthread_kill(etatJeu.chenillesG[2].tid, SIGUSR1);
                                    }

                                    if(etatJeu.chenillesG[3].presence == NORMAL && etatJeu.etatAmis[FLEUR_HG] != TOUCHE){
                                        etatJeu.score++;

                                        // idem
                                        pthread_kill(etatJeu.chenillesG[3].tid, SIGUSR1);
                                    }
                                }

                                if(etatJeu.positionStanley == 0){
                                    if(etatJeu.chenillesG[0].presence == NORMAL && etatJeu.etatAmis[FLEUR_HG] != TOUCHE){
                                        etatJeu.score++;

                                        // idem
                                        pthread_kill(etatJeu.chenillesG[0].tid, SIGUSR1);
                                    }

                                    if(etatJeu.chenillesG[1].presence == NORMAL && etatJeu.etatAmis[FLEUR_HG] != TOUCHE){
                                        etatJeu.score++;

                                        // idem
                                        pthread_kill(etatJeu.chenillesG[1].tid, SIGUSR1);
                                    }
                                }

                                if(etatJeu.positionStanley == 3){
                                    if(etatJeu.chenillesD[1].presence == NORMAL && etatJeu.etatAmis[FLEUR_HD] != TOUCHE){
                                        etatJeu.score++;

                                        // envoi du signal SIGUSR1 à ThreadChenilleD pour tuer la chenille
                                        pthread_kill(etatJeu.chenillesD[1].tid, SIGUSR1);
                                    }

                                    if(etatJeu.chenillesD[2].presence == NORMAL && etatJeu.etatAmis[FLEUR_HD] != TOUCHE){
                                        etatJeu.score++;

                                        // idem
                                        pthread_kill(etatJeu.chenillesD[2].tid, SIGUSR1);
                                    }
                                }

                                if(etatJeu.positionStanley == 4){
                                    if(etatJeu.chenillesD[3].presence == NORMAL && etatJeu.etatAmis[FLEUR_HD] != TOUCHE){
                                        etatJeu.score++;

                                        // idem
                                        pthread_kill(etatJeu.chenillesD[3].tid, SIGUSR1);
                                    }

                                    if(etatJeu.chenillesD[4].presence == NORMAL && etatJeu.etatAmis[FLEUR_HD] != TOUCHE){
                                        etatJeu.score++;

                                        // idem
                                        pthread_kill(etatJeu.chenillesD[4].tid, SIGUSR1);
                                    }
                                }

                                if(etatJeu.positionStanley == 5){
                                    if(etatJeu.chenillesD[5].presence == NORMAL && etatJeu.etatAmis[FLEUR_HD] != TOUCHE){
                                        etatJeu.score++;

                                        // idem
                                        pthread_kill(etatJeu.chenillesD[5].tid, SIGUSR1);
                                    }

                                    if(etatJeu.chenillesD[6].presence == NORMAL && etatJeu.etatAmis[FLEUR_HD] != TOUCHE){
                                        etatJeu.score++;

                                        // idem
                                        pthread_kill(etatJeu.chenillesD[6].tid, SIGUSR1);
                                    }
                                }
                            }

                            break;
                                
                        case SDLK_LEFT:
                            if(etatJeu.positionStanley > 0){
                                etatJeu.positionStanley--;
                            }

                            break;

                        case SDLK_RIGHT:
                            if(etatJeu.positionStanley < 5){
                                etatJeu.positionStanley++;
                            }

                            break;

                        case SDLK_DOWN:
                            if(etatJeu.positionStanley == 2){
                                etatJeu.etatStanley = ECHELLE;
                                etatJeu.positionStanley = 0;
                            }

                            break;
                    }

                    break;

                case ECHELLE:
                    switch(evenement){
                        case SDLK_UP:
                            if(etatJeu.positionStanley == 0){
                                etatJeu.etatStanley = HAUT;
                                etatJeu.positionStanley = 2;
                            }

                            else{
                                etatJeu.positionStanley--;
                            }

                            break;

                        case SDLK_DOWN:
                            if(etatJeu.positionStanley == 1){
                                etatJeu.etatStanley = BAS;
                            }

                            else{
                                etatJeu.positionStanley++;
                            }

                            break;
                    }

                    break;
            }

            pthread_mutex_unlock(&mutexEtatJeu);
            
            evenement = AUCUN;
            
            pthread_mutex_unlock(&mutexEvenement);
        }

        else{
            pthread_mutex_unlock(&mutexEchec);
        }
    }

    pthread_exit(0);
}

void *fctThreadEnnemis(void *param){
    // bloque tous les signaux sauf SIGALRM
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGALRM);
    sigprocmask(SIG_SETMASK, &set, NULL);

    int *delai = (int *)malloc(sizeof(int));
    *delai = DELAI_MAX;
    
    pthread_setspecific(keySpec, delai);

    // premier envoi du signal SIGALRM
    alarm(10);

    while(true){
        usleep(*delai);

        int typeEnnemi = rand() % 5;

        pthread_mutex_lock(&mutexEchec);

        if(echec == AUCUN){
            pthread_mutex_unlock(&mutexEchec);

            switch(typeEnnemi){
                case GUEPE:
                    pthread_create(&threadEnnemi, NULL, fctThreadGuepe, NULL);
                    break;
                case CHENILLE_G:
                    pthread_create(&threadEnnemi, NULL, fctThreadChenilleG, NULL);
                    break;
                case CHENILLE_D:
                    pthread_create(&threadEnnemi, NULL, fctThreadChenilleD, NULL);
                    break;
                case ARAIGNEE_G:
                    pthread_create(&threadEnnemi, NULL, fctThreadAraigneeG, NULL);
                    break;
                case ARAIGNEE_D:
                    pthread_create(&threadEnnemi, NULL, fctThreadAraigneeD, NULL);
                    break;
                default:
                    break;
            }
        }

        else{
            pthread_mutex_unlock(&mutexEchec);
        }
    }
    
    pthread_exit(0);
}

void *fctThreadGuepe(void *param){
    // bloque tous les signaux sauf SIGINT
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGINT);
    sigprocmask(SIG_SETMASK, &set, NULL);

    int *position = (int *)malloc(sizeof(int));
    if(position == NULL){
        pthread_exit(0);
    }

    *position = 0;

    pthread_setspecific(keySpec, position);

    pthread_mutex_lock(&mutexEtatJeu);
    etatJeu.guepes[0].presence = NORMAL;
    etatJeu.guepes[0].tid = pthread_self();
    pthread_mutex_unlock(&mutexEtatJeu);

    while(true){
        usleep(1000000);

        pthread_mutex_lock(&mutexEchec);

        if(echec == AUCUN){
            pthread_mutex_unlock(&mutexEchec);

            pthread_mutex_lock(&mutexEtatJeu);

            if(etatJeu.etatStanley == BAS && etatJeu.positionStanley == 2 && etatJeu.actionStanley == SPRAY){
                etatJeu.guepes[*position].presence = AUCUN;
                etatJeu.score++;
                pthread_mutex_unlock(&mutexEtatJeu);
                pthread_exit(0);
            }

            // la gupe pique le chat
            if(*position >= 1){
                pthread_mutex_lock(&mutexEchec);
                echec = CHAT;
                pthread_mutex_unlock(&mutexEchec);

                pthread_cond_signal(&condEchec);

                pthread_mutex_unlock(&mutexEtatJeu);

                usleep(1500000);

                pthread_mutex_lock(&mutexEtatJeu);
                etatJeu.guepes[1].presence = AUCUN;
                etatJeu.guepes[1].tid = 0;
                pthread_mutex_unlock(&mutexEtatJeu);

                pthread_exit(0);
            }
            
            // déplacement de la guêpe
            else{
                etatJeu.guepes[1].presence = NORMAL;
                etatJeu.guepes[1].tid = pthread_self();
                etatJeu.guepes[0].presence = AUCUN;
                etatJeu.guepes[0].tid = AUCUN;

                (*position)++;
            }

            pthread_mutex_unlock(&mutexEtatJeu);
        }

        else{
            pthread_mutex_unlock(&mutexEchec);
        }
    }

    pthread_exit(0);
}

void *fctThreadChenilleG(void *param){
    // bloque tous les signaux sauf SIGUSR1
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGUSR1);
    sigprocmask(SIG_SETMASK, &set, NULL);

    S_LOCALISATION *positionChenilleG = (S_LOCALISATION *)malloc(sizeof(S_LOCALISATION));
    if(positionChenilleG == NULL)
        pthread_exit(0);
    
    positionChenilleG->position = 4;
    positionChenilleG->orientation = GAUCHE;

    pthread_setspecific(keySpec, positionChenilleG);

    pthread_mutex_lock(&mutexEtatJeu);
    etatJeu.chenillesG[positionChenilleG->position].presence = NORMAL;
    etatJeu.chenillesG[positionChenilleG->position].tid = pthread_self();
    pthread_mutex_unlock(&mutexEtatJeu);

    while(true){
        usleep(800000);

        pthread_mutex_lock(&mutexEchec);

        if(echec == AUCUN){
            pthread_mutex_unlock(&mutexEchec);

            pthread_mutex_lock(&mutexEtatJeu);

            if(etatJeu.etatStanley == HAUT && etatJeu.actionStanley == SPRAY &&
                ((etatJeu.positionStanley == 0 && (positionChenilleG->position == 0 || positionChenilleG->position == 1)) ||
                (etatJeu.positionStanley == 1 && (positionChenilleG->position == 2 || positionChenilleG->position == 3))
            )){
                
                etatJeu.score++;
                etatJeu.chenillesG[positionChenilleG->position].presence = AUCUN;
                etatJeu.chenillesG[positionChenilleG->position].tid = 0;

                pthread_mutex_unlock(&mutexEtatJeu);
                pthread_exit(0);
            }

            // la chenille mange la fleur
            if(positionChenilleG->position <= 0){
                pthread_mutex_lock(&mutexEchec);
                echec = FLEUR_HG;
                pthread_mutex_unlock(&mutexEchec);

                pthread_cond_signal(&condEchec);

                pthread_mutex_unlock(&mutexEtatJeu);

                usleep(1500000);

                pthread_mutex_lock(&mutexEtatJeu);
                etatJeu.chenillesG[0].presence = AUCUN;
                etatJeu.chenillesG[0].tid = 0;
                pthread_mutex_unlock(&mutexEtatJeu);

                pthread_exit(0);
            }

            // déplacement de la chenille
            else{
                etatJeu.chenillesG[positionChenilleG->position - 1].presence = NORMAL;
                etatJeu.chenillesG[positionChenilleG->position - 1].tid = pthread_self();
                etatJeu.chenillesG[positionChenilleG->position].presence = AUCUN;
                etatJeu.chenillesG[positionChenilleG->position].tid = 0;

                positionChenilleG->position--;
            }

            pthread_mutex_unlock(&mutexEtatJeu);
        }

        else{
            pthread_mutex_unlock(&mutexEchec);
        }
    }

    pthread_exit(0);
}

void *fctThreadChenilleD(void *param){
    // bloque tous les signaux sauf SIGUSR1
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGUSR1);
    sigprocmask(SIG_SETMASK, &set, NULL);
    
    S_LOCALISATION *positionChenilleD = (S_LOCALISATION *)malloc(sizeof(S_LOCALISATION));
    if(positionChenilleD == NULL)
        pthread_exit(0);
    
    positionChenilleD->position = 0;
    positionChenilleD->orientation = DROITE;

    pthread_setspecific(keySpec, positionChenilleD);

    pthread_mutex_lock(&mutexEtatJeu);
    etatJeu.chenillesD[positionChenilleD->position].presence = NORMAL;
    etatJeu.chenillesD[positionChenilleD->position].tid = pthread_self();
    pthread_mutex_unlock(&mutexEtatJeu);

    while(true){
        usleep(800000);

        pthread_mutex_lock(&mutexEchec);

        if(echec == AUCUN){
            pthread_mutex_unlock(&mutexEchec);

            pthread_mutex_lock(&mutexEtatJeu);

            if(etatJeu.etatStanley == HAUT && etatJeu.actionStanley == SPRAY &&
                ((etatJeu.positionStanley == 3 && (positionChenilleD->position == 1 || positionChenilleD->position == 2)) ||
                (etatJeu.positionStanley == 4 && (positionChenilleD->position == 3 || positionChenilleD->position == 4)) ||
                (etatJeu.positionStanley == 5 && (positionChenilleD->position == 5 || positionChenilleD->position == 6))
            )){

                etatJeu.score++;
                etatJeu.chenillesD[positionChenilleD->position].presence = AUCUN;
                etatJeu.chenillesD[positionChenilleD->position].tid = 0;

                pthread_mutex_unlock(&mutexEtatJeu);
                pthread_exit(0);
            }

            // la chenille mange la fleur
            if(positionChenilleD->position >= 6){
                pthread_mutex_lock(&mutexEchec);
                echec = FLEUR_HD;
                pthread_mutex_unlock(&mutexEchec);

                pthread_cond_signal(&condEchec);

                pthread_mutex_unlock(&mutexEtatJeu);

                usleep(1500000);

                pthread_mutex_lock(&mutexEtatJeu);
                etatJeu.chenillesD[6].presence = AUCUN;
                etatJeu.chenillesD[6].tid = 0;
                pthread_mutex_unlock(&mutexEtatJeu);

                pthread_exit(0);
            }

            // déplacement de la chenille
            else{
                etatJeu.chenillesD[positionChenilleD->position + 1].presence = NORMAL;
                etatJeu.chenillesD[positionChenilleD->position + 1].tid = pthread_self();
                etatJeu.chenillesD[positionChenilleD->position].presence = AUCUN;
                etatJeu.chenillesD[positionChenilleD->position].tid = 0;

                positionChenilleD->position++;
            }

            pthread_mutex_unlock(&mutexEtatJeu);
        }

        else{
            pthread_mutex_unlock(&mutexEchec);
        }
    }

    pthread_exit(0);
}

void *fctThreadAraigneeG(void *param){
    // bloque tous les signaux sauf SIGUSR2
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGUSR2);
    sigprocmask(SIG_SETMASK, &set, NULL);

    S_LOCALISATION *positionAraigneeG = (S_LOCALISATION *)malloc(sizeof(S_LOCALISATION));
    if(positionAraigneeG == NULL)
        pthread_exit(0);
    
    positionAraigneeG->position = 0;
    positionAraigneeG->orientation = GAUCHE;

    pthread_setspecific(keySpec, positionAraigneeG);

    pthread_mutex_lock(&mutexEtatJeu);
    etatJeu.araigneesG[positionAraigneeG->position].presence = NORMAL;
    etatJeu.araigneesG[positionAraigneeG->position].tid = pthread_self();
    pthread_mutex_unlock(&mutexEtatJeu);

    while(true){
        usleep(600000);

        pthread_mutex_lock(&mutexEchec);
        
        if(echec == AUCUN){
            pthread_mutex_unlock(&mutexEchec);

            pthread_mutex_lock(&mutexEtatJeu);

            // de l'insecticide se trouve à la position de l'araignée, on tue le nuage d'insecticide en envoyant le signal SIGQUIT à ThreadInsecticideG
            if(etatJeu.insecticidesG[positionAraigneeG->position].presence == NORMAL){
                pthread_kill(etatJeu.insecticidesG[positionAraigneeG->position].tid, SIGQUIT);
                etatJeu.araigneesG[positionAraigneeG->position].presence = AUCUN;
                etatJeu.araigneesG[positionAraigneeG->position].tid = 0;
                etatJeu.score++;
                pthread_mutex_unlock(&mutexEtatJeu);

                pthread_exit(0);
            }

            if(positionAraigneeG->position >= 4){
                // Stanley tue l'araignée sans avoir besoin de créer un thread insecticide
                if(etatJeu.etatStanley == BAS && etatJeu.positionStanley == 0 && etatJeu.actionStanley == SPRAY){
                    etatJeu.araigneesG[positionAraigneeG->position].presence = AUCUN;
                    etatJeu.araigneesG[positionAraigneeG->position].tid = 0;
                    etatJeu.score++;
                }

                // l'araignée mange la fleur
                else{
                    pthread_mutex_lock(&mutexEchec);
                    echec = FLEUR_BG;
                    pthread_mutex_unlock(&mutexEchec);

                    pthread_cond_signal(&condEchec);

                    pthread_mutex_unlock(&mutexEtatJeu);

                    usleep(1500000);

                    pthread_mutex_lock(&mutexEtatJeu);
                    etatJeu.araigneesG[4].presence = AUCUN;
                    etatJeu.araigneesG[4].tid = 0;
                }

                pthread_mutex_unlock(&mutexEtatJeu);
                pthread_exit(0);
            }

            // déplacement de l'araignée
            else{
                etatJeu.araigneesG[positionAraigneeG->position + 1].presence = NORMAL;
                etatJeu.araigneesG[positionAraigneeG->position + 1].tid = pthread_self();
                etatJeu.araigneesG[positionAraigneeG->position].presence = AUCUN;
                etatJeu.araigneesG[positionAraigneeG->position].tid = 0;

                positionAraigneeG->position++;
            }

            pthread_mutex_unlock(&mutexEtatJeu);
        }

        else{
            pthread_mutex_unlock(&mutexEchec);
        }
    }

    pthread_exit(0);
}

void *fctThreadAraigneeD(void *param){
    // bloque tous les signaux sauf SIGUSR2
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGUSR2);
    sigprocmask(SIG_SETMASK, &set, NULL);

    S_LOCALISATION *positionAraigneeD = (S_LOCALISATION *)malloc(sizeof(S_LOCALISATION));
    if(positionAraigneeD == NULL)
        pthread_exit(0);
    
    positionAraigneeD->position = 4;
    positionAraigneeD->orientation = DROITE;

    pthread_setspecific(keySpec, positionAraigneeD);

    etatJeu.araigneesD[positionAraigneeD->position].presence = NORMAL;
    etatJeu.araigneesD[positionAraigneeD->position].tid = pthread_self();

    while(true){
        usleep(600000);

        pthread_mutex_lock(&mutexEchec);
        
        if(echec == AUCUN){
            pthread_mutex_unlock(&mutexEchec);

            pthread_mutex_lock(&mutexEtatJeu);

            // de l'insecticide se trouve à la position de l'araignée, on tue le nuage d'insecticide en envoyant le signal SIGQUIT à ThreadInsecticideD
            if(etatJeu.insecticidesD[positionAraigneeD->position - 1].presence == NORMAL){
                pthread_kill(etatJeu.insecticidesD[positionAraigneeD->position - 1].tid, SIGQUIT);
                etatJeu.araigneesD[positionAraigneeD->position].presence = AUCUN;
                etatJeu.araigneesD[positionAraigneeD->position].tid = 0;
                etatJeu.score++;
                pthread_mutex_unlock(&mutexEtatJeu);

                pthread_exit(0);
            }

            if(positionAraigneeD->position <= 0){
                // Stanley tue l'araignée sans avoir besoin de créer un thread insecticide
                if(etatJeu.etatStanley == BAS && etatJeu.positionStanley == 3 && etatJeu.actionStanley == SPRAY) {
                    etatJeu.araigneesD[positionAraigneeD->position].presence = AUCUN;
                    etatJeu.araigneesD[positionAraigneeD->position].tid = 0;
                    etatJeu.score++;
                }

                // l'araignée mange la fleur
                else{
                    pthread_mutex_lock(&mutexEchec);
                    echec = FLEUR_BD;
                    pthread_mutex_unlock(&mutexEchec);

                    pthread_cond_signal(&condEchec);

                    pthread_mutex_unlock(&mutexEtatJeu);

                    usleep(1500000);

                    pthread_mutex_lock(&mutexEtatJeu);
                    etatJeu.araigneesD[0].presence = AUCUN;
                    etatJeu.araigneesD[0].tid = 0;
                }

                pthread_mutex_unlock(&mutexEtatJeu);
                pthread_exit(0);
            }

            // déplacement de l'araignée
            else{
                etatJeu.araigneesD[positionAraigneeD->position - 1].presence = NORMAL;
                etatJeu.araigneesD[positionAraigneeD->position - 1].tid = pthread_self();
                etatJeu.araigneesD[positionAraigneeD->position].presence = AUCUN;
                etatJeu.araigneesD[positionAraigneeD->position].tid = 0;

                positionAraigneeD->position--;
            }

            pthread_mutex_unlock(&mutexEtatJeu);

        }

        else{
            pthread_mutex_unlock(&mutexEchec);
        }
    }

    pthread_exit(0);
}

void *fctThreadInsecticideG(void *param){
    // bloque tous les signaux sauf SIGQUIT
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGQUIT);
    sigprocmask(SIG_SETMASK, &set, NULL);

    S_LOCALISATION *positionInsecticideG = (S_LOCALISATION *)malloc(sizeof(S_LOCALISATION));
    if(positionInsecticideG == NULL)
        pthread_exit(0);
    
    positionInsecticideG->position = 3;
    positionInsecticideG->orientation = GAUCHE;

    pthread_setspecific(keySpec, positionInsecticideG);

    pthread_mutex_lock(&mutexEtatJeu);
    etatJeu.insecticidesG[3].presence = NORMAL;
    etatJeu.insecticidesG[3].tid = pthread_self();
    pthread_mutex_unlock(&mutexEtatJeu);

    while(true){
        usleep(200000);

        pthread_mutex_lock(&mutexEchec);

        if(echec == AUCUN){
            pthread_mutex_unlock(&mutexEchec);

            pthread_mutex_lock(&mutexEtatJeu);

            // une araignée est présente dans le nuage d'insecticide, on tue l'araignée en envoyant le signal SIGUSR2 à ThreadAraigneeG
            if(etatJeu.araigneesG[positionInsecticideG->position].presence == NORMAL){
                pthread_kill(etatJeu.araigneesG[positionInsecticideG->position].tid, SIGUSR2);
                etatJeu.insecticidesG[positionInsecticideG->position].presence = AUCUN;
                etatJeu.insecticidesG[positionInsecticideG->position].tid = 0;
                etatJeu.score++;
                pthread_mutex_unlock(&mutexEtatJeu);

                pthread_exit(0);
            }

            // disparition de l'insecticide
            if(positionInsecticideG->position <= 0){
                etatJeu.insecticidesG[0].presence = AUCUN;
                etatJeu.insecticidesG[0].tid = 0;
                pthread_mutex_unlock(&mutexEtatJeu);

                pthread_exit(0);
            }

            // déplacement de l'insecticide
            else{
                etatJeu.insecticidesG[positionInsecticideG->position - 1].presence = NORMAL;
                etatJeu.insecticidesG[positionInsecticideG->position - 1].tid = pthread_self();
                etatJeu.insecticidesG[positionInsecticideG->position].presence = AUCUN;
                etatJeu.insecticidesG[positionInsecticideG->position].tid = 0;

                positionInsecticideG->position--;
            }

            pthread_mutex_unlock(&mutexEtatJeu);
        }

        else{
            pthread_mutex_unlock(&mutexEchec);
        }
    }

    pthread_exit(0);
}

void *fctThreadInsecticideD(void *param){
    // bloque tous les signaux sauf SIGQUIT
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGQUIT);
    sigprocmask(SIG_SETMASK, &set, NULL);

    S_LOCALISATION *positionInsecticideD = (S_LOCALISATION *)malloc(sizeof(S_LOCALISATION));
    if(positionInsecticideD == NULL)
        pthread_exit(0);
    
    positionInsecticideD->position = 1;
    positionInsecticideD->orientation = DROITE;

    pthread_setspecific(keySpec, positionInsecticideD);

    pthread_mutex_lock(&mutexEtatJeu);
    etatJeu.insecticidesD[positionInsecticideD->position - 1].presence = NORMAL;
    etatJeu.insecticidesD[positionInsecticideD->position - 1].tid = pthread_self();
    pthread_mutex_unlock(&mutexEtatJeu);

    while(true){
        usleep(200000);

        pthread_mutex_lock(&mutexEchec);

        if(echec == AUCUN){
            pthread_mutex_unlock(&mutexEchec);

            pthread_mutex_lock(&mutexEtatJeu);

            // une araignée est présente dans le nuage d'insecticide, on tue l'araignée en envoyant le signal SIGUSR2 à ThreadAraigneeD
            if(etatJeu.araigneesD[positionInsecticideD->position].presence == NORMAL){
                pthread_kill(etatJeu.araigneesD[positionInsecticideD->position].tid, SIGUSR2);
                etatJeu.insecticidesD[positionInsecticideD->position - 1].presence = AUCUN;
                etatJeu.insecticidesD[positionInsecticideD->position - 1].tid = 0;
                etatJeu.score++;
                pthread_mutex_unlock(&mutexEtatJeu);

                pthread_exit(0);
            }

            // disparition de l'insecticide
            if(positionInsecticideD->position >= 4){
                etatJeu.insecticidesD[3].presence = AUCUN;
                etatJeu.insecticidesD[3].tid = 0;
                pthread_mutex_unlock(&mutexEtatJeu);

                pthread_exit(0);
            }

            // déplacement de l'insecticide
            else{
                etatJeu.insecticidesD[positionInsecticideD->position].presence = NORMAL;
                etatJeu.insecticidesD[positionInsecticideD->position].tid = pthread_self();
                etatJeu.insecticidesD[positionInsecticideD->position - 1].presence = AUCUN;
                etatJeu.insecticidesD[positionInsecticideD->position - 1].tid = 0;

                positionInsecticideD->position++;
            }

            pthread_mutex_unlock(&mutexEtatJeu);
        }

        else{
            pthread_mutex_unlock(&mutexEchec);
        }
    }

    pthread_exit(0);
}

void handlerSIGALRM(int sig){
    int *delai = (int *)pthread_getspecific(keySpec);
    *delai = DELAI_MIN + (rand() % ((DELAI_MAX - DELAI_MIN + 1) / 100000)) * 100000;
    
    alarm(10);
}

void handlerSIGINT(int sig){
    int *positionGuepe = (int *)pthread_getspecific(keySpec);
    
    pthread_mutex_lock(&mutexEtatJeu);
    etatJeu.guepes[*positionGuepe].presence = AUCUN;
    etatJeu.guepes[*positionGuepe].tid = 0;
    pthread_mutex_unlock(&mutexEtatJeu);
    
    pthread_exit(0);
}

void handlerSIGUSR1(int sig){
    S_LOCALISATION *positionChenille = (S_LOCALISATION *)pthread_getspecific(keySpec);

    pthread_mutex_lock(&mutexEtatJeu);
    
    if(positionChenille->orientation == GAUCHE){
        etatJeu.chenillesG[positionChenille->position].presence = AUCUN;
        etatJeu.chenillesG[positionChenille->position].tid = 0;
    }
    else{
        etatJeu.chenillesD[positionChenille->position].presence = AUCUN;
        etatJeu.chenillesD[positionChenille->position].tid = 0;
    }
    
    pthread_mutex_unlock(&mutexEtatJeu);

    pthread_exit(0);
}

void handlerSIGUSR2(int sig){
    S_LOCALISATION *positionAraignee = (S_LOCALISATION *)pthread_getspecific(keySpec);

    pthread_mutex_lock(&mutexEtatJeu);
    
    if(positionAraignee->orientation == GAUCHE){
        etatJeu.araigneesG[positionAraignee->position].presence = AUCUN;
        etatJeu.araigneesG[positionAraignee->position].tid = 0;
    }
    else{
        etatJeu.araigneesD[positionAraignee->position].presence = AUCUN;
        etatJeu.araigneesD[positionAraignee->position].tid = 0;
    }
    
    pthread_mutex_unlock(&mutexEtatJeu);

    pthread_exit(0);
}

void handlerSIGQUIT(int sig){
    S_LOCALISATION *positionInsecticide = (S_LOCALISATION *)pthread_getspecific(keySpec);

    pthread_mutex_lock(&mutexEtatJeu);
    
    if(positionInsecticide->orientation == GAUCHE){
        etatJeu.insecticidesG[positionInsecticide->position].presence = AUCUN;
        etatJeu.insecticidesG[positionInsecticide->position].tid = 0;
    }
    else{
        etatJeu.insecticidesD[positionInsecticide->position - 1].presence = AUCUN;
        etatJeu.insecticidesD[positionInsecticide->position - 1].tid = 0;
    }
    
    pthread_mutex_unlock(&mutexEtatJeu);

    pthread_exit(0);
}

void destructeurVS(void *p){
    free(p);
}

int main(int argc, char* argv[]){
    sigset_t set;
    sigfillset(&set);
    sigprocmask(SIG_SETMASK, &set, NULL);

    ouvrirFenetreGraphique();

    signal(SIGALRM, handlerSIGALRM);
    signal(SIGUSR1, handlerSIGUSR1);
    signal(SIGUSR2, handlerSIGUSR2);
    signal(SIGINT, handlerSIGINT);
    signal(SIGQUIT, handlerSIGQUIT);

    pthread_cond_init(&condEvenement, NULL);
    pthread_cond_init(&condEchec, NULL);

    pthread_mutex_init(&mutexEtatJeu, NULL);
    pthread_mutex_init(&mutexEvenement, NULL);
    pthread_mutex_init(&mutexEchec, NULL);

    pthread_key_create(&keySpec, destructeurVS);

    pthread_create(&threadFenetreGraphique, NULL, fctThreadFenetreGraphique, NULL);
    pthread_create(&threadEvenement, NULL, fctThreadEvenements, NULL);
    pthread_create(&threadStanley, NULL, fctThreadStanley, NULL);
    pthread_create(&threadEnnemis, NULL, fctThreadEnnemis, NULL);

    while(etatJeu.nbEchecs < 3){
        pthread_mutex_lock(&mutexEchec);
        while(echec == AUCUN){
            pthread_cond_wait(&condEchec, &mutexEchec);
        }

        pthread_mutex_lock(&mutexEtatJeu);
        etatJeu.nbEchecs++;
        
        switch(echec){
            case CHAT:
                etatJeu.etatAmis[CHAT] = TOUCHE;
                break;
            case FLEUR_HG:
                etatJeu.etatAmis[FLEUR_HG] = TOUCHE;
                break;
            case FLEUR_HD:
                etatJeu.etatAmis[FLEUR_HD] = TOUCHE;
                break;
            case FLEUR_BG:
                etatJeu.etatAmis[FLEUR_BG] = TOUCHE;
                break;
            case FLEUR_BD:
                etatJeu.etatAmis[FLEUR_BD] = TOUCHE;
                break;
            default:
                break;
        }

        if(etatJeu.nbEchecs < 3){
            pthread_mutex_unlock(&mutexEtatJeu);

            usleep(1500000);

            pthread_mutex_lock(&mutexEtatJeu);
            
            switch(echec){
                case CHAT:
                    etatJeu.etatAmis[CHAT] = NORMAL;
                    break;
                case FLEUR_HG:
                    etatJeu.etatAmis[FLEUR_HG] = NORMAL;
                    break;
                case FLEUR_HD:
                    etatJeu.etatAmis[FLEUR_HD] = NORMAL;
                    break;
                case FLEUR_BG:
                    etatJeu.etatAmis[FLEUR_BG] = NORMAL;
                    break;
                case FLEUR_BD:
                    etatJeu.etatAmis[FLEUR_BD] = NORMAL;
                    break;
                default:
                    break;
            }

            echec = AUCUN;
            pthread_mutex_unlock(&mutexEchec);
        }
        
        pthread_mutex_unlock(&mutexEtatJeu);
    }

    pthread_mutex_destroy(&mutexEvenement);
    pthread_mutex_destroy(&mutexEtatJeu);
    pthread_mutex_destroy(&mutexEchec);

    pthread_cond_destroy(&condEvenement);
    pthread_cond_destroy(&condEchec);

    pthread_join(threadEvenement, NULL);

    return 0;
}