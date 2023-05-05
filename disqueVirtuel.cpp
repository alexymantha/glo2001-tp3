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

    void DisqueVirtuel::creerRepertoireVide(Block *monBlock) {

        monBlock->m_type_donnees = S_IFIN;

        iNode *inode = monBlock->m_inode;
        inode->st_mode = S_IFDIR;
        inode->st_nlink = 1;
        inode->st_size = 28;

        monBlock->m_dirEntry = std::vector<dirEntry *>(2);

        dirEntry self = dirEntry(inode->st_ino, ".");
        dirEntry parent = dirEntry(inode->st_ino - 1, ".."); //TODO Est-ce la bonne manière d'aller chercher le inode parent?

        monBlock->m_dirEntry[0] = &self;
        monBlock->m_dirEntry[1] = &parent;

    }

    bool DisqueVirtuel::repertoireExiste() {
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
			m_blockDisque[i].m_inode = new iNode(i - 4, 0, 0, 0, 0);
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

    int bd_mkdir(const std::string& p_DirName) {

        //Avec "/doc/tmp/test", il faudra valider si doc et tmp existent avant d'ajouter test

        std::string str1 ("/usr/bin/man");
        std::string str2 ("c:\\windows\\winhelp.exe");

        std::size_t found = str1.find_last_of("/");
        std::string path = str1.substr(0,found);
        std::string file = str1.substr(found+1);



    }

} // Fin du namespace
