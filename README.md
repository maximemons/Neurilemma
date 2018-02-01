3.2.1 Test du programme :

  1. Quitter votre serveur (Ctrl+C) et relancez le. 
     Que se passe-t-il ?
    La socket est maintenue par défaut, i.e. on ne peut plus s'y lier
    -----
    bind socker_serveur: Address already in use
     accept : Bad file descriptor
    -----

  2. Ajouter un petit délai avant l'envoi du message de bienvenue, 
     puis exécutez la commande nc -z 127.0.0.1 8080.
     Que se passe-t-il ?
    La commande nc -z 127.0.0.1 8080 fait planter le serveur
    Le délai empéche la reception des messages. 
  
  4. Lancez deux clients simultanément.
     Que se passe-t-il ?
     Pourquoi ?
    Les deux clients reçoivent le message.
