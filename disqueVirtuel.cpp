/**
 * \file disqueVirtuel.cpp
 * \brief Implémentation d'un disque virtuel.
 * \author ?
 * \version 0.1
 * \date  2022
 *
 *  Travail pratique numéro 3
 *
 */

#include "disqueVirtuel.h"
#include <string>
#include <sstream>

namespace TP3
{

    //Méthodes utilitaires

    int DisqueVirtuel::premierINodeLibre() {

        int premierINodeLibre = 0;
        while (!m_blockDisque[FREE_INODE_BITMAP].m_bitmap[premierINodeLibre]) {
            premierINodeLibre++;
            if(premierINodeLibre == N_INODE_ON_DISK) return 0; //TODO Gérer ce cas d'erreur
        }
        return premierINodeLibre;
    }

    int DisqueVirtuel::premierBlocLibre() {

        int premierBlocLibre = 24;
        while (!m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[premierBlocLibre]) {
            premierBlocLibre++;
            if(premierBlocLibre == N_INODE_ON_DISK) return 0; //TODO Gérer ce cas d'erreur
        }
        return premierBlocLibre;
    }

    void DisqueVirtuel::reserverINode(int pos) {
        m_blockDisque[FREE_INODE_BITMAP].m_bitmap.at(pos) = false;
    }

    void DisqueVirtuel::libereriNode(int pos) {
        m_blockDisque[FREE_INODE_BITMAP].m_bitmap.at(pos) = true;
    }

    void DisqueVirtuel::ajouterRepertoireVide(Block *blockParent, std::string nomRepertoire) {

        //On réserve un inode auprès de FREE_INODE_BITMAP
        Block *newBlock = &m_blockDisque[premierINodeLibre() + 4];
        reserverINode(premierINodeLibre());

        //Les métadonnées du nouveau répertoire sont configurées
        iNode *newInode = newBlock->m_inode;
        newInode->st_mode = S_IFDIR;
        newInode->st_nlink = 1;
        newInode->st_size = 28;

        //Les deux répertoires "." et ".." sont ajoutés au nouveau répertoire
        newBlock->m_dirEntry = std::vector<dirEntry *>(2);
        dirEntry self = dirEntry(newInode->st_ino, ".");
        dirEntry parent = dirEntry(blockParent->m_inode->st_ino, "..");
        newBlock->m_dirEntry[0] = &self;
        newBlock->m_dirEntry[1] = &parent;

        //Le nouveau répertoire est ajouté au répertoire parent
        blockParent->m_dirEntry.push_back(new dirEntry(newInode->st_ino, nomRepertoire));
        //TODO Incrémenter les st_nlink des parents

    }

    Block* DisqueVirtuel::blockRepertoire(std::string path) {

        //TODO Valider le fonctionnement pour le split string,
        // Je me suis basé sur ce code : https://java2blog.com/split-string-space-cpp/

        //On split le string selon le caractère "/"
        // Si path == "/doc/tmp/test"
        // Alors directories == ["doc", "tmp", "test"]
        std::vector<std::string> directories;
        std::stringstream ss(path);
        std::string tempo;
        while (std::getline(ss, tempo, '/')) {
            directories.push_back(tempo);
        }

        std::vector<dirEntry *> currentDir = m_blockDisque[5].m_dirEntry;
        int currentINode;
        for (int i = 0; i < directories.size(); i++) {

            bool exists = false;

            for(int j = 0; j < currentDir.size(); j++) {
                if(currentDir[j]->m_filename == directories.at(i)) {
                    currentINode = currentDir[j]->m_iNode;
                    currentDir = m_blockDisque[currentINode].m_dirEntry;
                    exists = true;
                    break;
                }
            }
            if(!exists) return NULL; //Le répertoire n'existe pas
        }
        return &m_blockDisque[currentINode]; //Le répertoire existe, retourne le block du chemin fourni
    }

    /*
    À VOIR
        Nous vous conseillons d’ajouter d’autres méthodes utilitaires, mais elles ne sont pas obligatoires. Il s’agit par
        exemple de méthodes permettant d’incrémenter ou de décrémenter le n-link dans l’i-node, d’augmenter ou de
        diminuer la taille inscrite dans l’i-node, de relâcher ou de saisir un bloc ou un i-node dans le
        FREE_BLOCK_BITMAP ou dans le FREE_INODE_BITMAP, etc.
     */

    //Méthodes principales à implémenter
	int DisqueVirtuel::bd_FormatDisk()
	{
		m_blockDisque = std::vector<Block>(N_BLOCK_ON_DISK);

		// Population de tous les blocs
		for (int i = 0; i < N_BLOCK_ON_DISK; i++)
		{
			m_blockDisque[i] = Block();
		}

		// Bloc 2 : bitmap des blocs libres
		m_blockDisque[FREE_BLOCK_BITMAP].m_type_donnees = S_IFBL;
		m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap = std::vector<bool>(N_BLOCK_ON_DISK, true);

        // Marquer touts les blocs de 0 à 23 comme non-libres
        for (int i = 0; i < 24; i++)
        {
            m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[i] = false;
        }

		// Bloc 3 : bitmap des inodes libres
		// Tous libres sauf le 0 qui est réservé.
		m_blockDisque[FREE_INODE_BITMAP].m_type_donnees = S_IFIL;
		m_blockDisque[FREE_INODE_BITMAP].m_bitmap = std::vector<bool>(N_INODE_ON_DISK, true);
		m_blockDisque[FREE_INODE_BITMAP].m_bitmap[0] = false;

		// Créer les inodes dans les blocs 4 à 23 (1 par bloc)
		for (int i = 4; i < 24; i++)
		{
			m_blockDisque[i].m_type_donnees = S_IFIN;
			m_blockDisque[i].m_inode = new iNode(i - 4, 0, 0, 0, i); //TODO Valider, j'ai modifié le champ b (0 --> i)
		}

		// Création de l'inode 1 pour le répertoire racine
		// 5eme bloc car on laisse le inode 0 libre
		m_blockDisque[5].m_type_donnees = S_IFIN;

		iNode *rootInode = m_blockDisque[5].m_inode;
		rootInode->st_mode = S_IFDIR;
		rootInode->st_nlink = 1;
		rootInode->st_size = 28;
		rootInode->st_block = 24;

		m_blockDisque[5].m_dirEntry = std::vector<dirEntry *>(1);
		dirEntry selfEntry = dirEntry(1, ".");
		m_blockDisque[5].m_dirEntry[0] = &selfEntry;

		return 1;
	}

    int DisqueVirtuel::bd_mkdir(const std::string& p_DirName) {

        //Exemple avec "/doc/tmp/test"
        //path est égal à /doc/tmp
        //directory est égal à test
        //On devra ajouter le répertoire test à /doc/tmp

        int found = p_DirName.find_last_of("/");
        std::string chemin = p_DirName.substr(0,found);
        std::string repertoire = p_DirName.substr(found+1);

        //Si le chemin d'accès est inexistant, on retourne 0
        if(blockRepertoire(chemin) == NULL) return 0;

        //Si le répertoire existe déjà, on retourne 0
        if(blockRepertoire(p_DirName) != NULL) return 0;

        //Sinon, on ajoute le répertoire au chemin
        ajouterRepertoireVide(blockRepertoire(chemin), repertoire);

        return 1;
    }

} // Fin du namespace
