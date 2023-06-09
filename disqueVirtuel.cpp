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
#include <iostream>
#include <iomanip>

namespace TP3
{

	DisqueVirtuel::DisqueVirtuel()
	{
		// Initialisation du disque
		m_blockDisque = std::vector<Block>(N_BLOCK_ON_DISK);
	}
	DisqueVirtuel::~DisqueVirtuel()
	{
		// Rien à faire, pas nécessaire de libérer la mémoire pour un vecteur
	}

	// Méthodes utilitaires
	int DisqueVirtuel::premierINodeLibre()
	{

		int inode = 0;
		while (!m_blockDisque[FREE_INODE_BITMAP].m_bitmap[inode])
		{
			inode++;
			if (inode == N_INODE_ON_DISK)
			{
				return 0;
			}
		}
		return inode;
	}

	int DisqueVirtuel::premierBlocLibre()
	{
		int block = FIRST_DATA_BLOCK;
		while (!m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[block])
		{
			block++;
			if (block == N_BLOCK_ON_DISK)
			{
				return 0;
			}
		}
		return block;
	}

	void DisqueVirtuel::reserverINode(int pos)
	{
		std::cout << "UFS: saisir i-node " << pos << std::endl;
		m_blockDisque[FREE_INODE_BITMAP].m_bitmap.at(pos) = false;
	}

	void DisqueVirtuel::libererINode(int pos)
	{
		std::cout << "UFS: relache i-node " << pos << std::endl;
		m_blockDisque[FREE_INODE_BITMAP].m_bitmap.at(pos) = true;
	}

	void DisqueVirtuel::reserverBlock(int pos)
	{
		std::cout << "UFS: saisir bloc " << pos << std::endl;
		m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap.at(pos) = false;
	}

	void DisqueVirtuel::libererBlock(int pos)
	{
		std::cout << "UFS: relache bloc " << pos << std::endl;
		m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap.at(pos) = true;
	}

	void DisqueVirtuel::ajouterRepertoireVide(Block *blockParent, std::string nomRepertoire)
	{

		// On réserve un inode auprès de FREE_INODE_BITMAP
		Block *newBlock = &m_blockDisque[premierINodeLibre() + 4];
		reserverINode(premierINodeLibre());

		// Reserver un bloc pour le nouveau répertoire
		int blockNo = premierBlocLibre();
		reserverBlock(blockNo);

		// Les métadonnées du nouveau répertoire sont configurées
		iNode *newInode = newBlock->m_inode;
		newInode->st_mode = S_IFDIR;
		newInode->st_nlink = 2; // Commence à 2 car "." et ".." sont ajoutés par défaut
		newInode->st_size = 56; // 56 octets pour "." et ".." de 28 octets chacun
		newInode->st_block = blockNo;

		// Les deux répertoires "." et ".." sont ajoutés au nouveau répertoire
		newBlock->m_dirEntry = std::vector<dirEntry *>(2);
		dirEntry *self = new dirEntry(newInode->st_ino, ".");
		dirEntry *parent = new dirEntry(blockParent->m_inode->st_ino, "..");
		newBlock->m_dirEntry[0] = self;
		newBlock->m_dirEntry[1] = parent;

		// Le nouveau répertoire est ajouté au répertoire parent
		blockParent->m_dirEntry.push_back(new dirEntry(newInode->st_ino, nomRepertoire));

		// Les métadonnées du répertoire parent sont mises à jour
		iNode *parentInode = blockParent->m_inode;
		parentInode->st_nlink++;
		parentInode->st_size += 28;
	}

	void DisqueVirtuel::ajouterFichierVide(Block *blockParent, std::string nomFichier)
	{
		Block *newBlock = &m_blockDisque[premierINodeLibre() + 4];
		reserverINode(premierINodeLibre());

		// Les métadonnées du nouveau fichier sont configurées
		iNode *newInode = newBlock->m_inode;
		newInode->st_mode = S_IFREG;
		newInode->st_nlink = 1;
		newInode->st_size = 0;
		newInode->st_block = -1;

		// Le nouveau fichier est ajouté au répertoire parent
		blockParent->m_dirEntry.push_back(new dirEntry(newInode->st_ino, nomFichier));
		blockParent->m_inode->st_size += 28;
	}

	/**
	 * \fn Block *DisqueVirtuel::getBlock(std::string chemin)
	 * \brief Retourne le block du chemin fourni, null si aucun block n'est trouvé.
	 */
	Block *DisqueVirtuel::getBlock(std::string chemin)
	{
		// On split le string selon le caractère "/"
		//  Si path == "/doc/tmp/test"
		//  Alors directories == ["doc", "tmp", "test"]
		std::vector<std::string> directories;
		std::stringstream ss(chemin);
		std::string tempo;
		while (std::getline(ss, tempo, '/'))
		{
			if (tempo == "")
			{
				continue;
			}
			directories.push_back(tempo);
		}

		std::vector<dirEntry *> dirs = m_blockDisque[5].m_dirEntry;
		int currentBlock = 5;
		for (int i = 0; i < directories.size(); i++)
		{

			bool exists = false;

			for (int j = 0; j < dirs.size(); j++)
			{
				if (dirs[j]->m_filename == directories.at(i))
				{
					currentBlock = dirs[j]->m_iNode + 4;
					dirs = m_blockDisque[currentBlock].m_dirEntry;
					exists = true;
					break;
				}
			}
			if (!exists)
				return NULL; // Le répertoire n'existe pas
		}
		return &m_blockDisque[currentBlock]; // Le répertoire existe, retourne le block du chemin fourni
	}

	// Méthodes principales à implémenter
	int DisqueVirtuel::bd_FormatDisk()
	{
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
		reserverINode(1);

		iNode *rootInode = m_blockDisque[5].m_inode;
		rootInode->st_mode = S_IFDIR;
		rootInode->st_nlink = 2; // 2 link car "." et ".."
		rootInode->st_size = 56; // 56 bytes car 2 dirEntry de 28 bytes

		m_blockDisque[5].m_dirEntry = std::vector<dirEntry *>(2);
		dirEntry *selfEntry = new dirEntry(rootInode->st_ino, ".");
		m_blockDisque[5].m_dirEntry[0] = selfEntry;

		dirEntry *parentEntry = new dirEntry(rootInode->st_ino, "..");
		m_blockDisque[5].m_dirEntry[1] = parentEntry;

		reserverBlock(FIRST_DATA_BLOCK); // Réserver le premier bloc pour le répertoire racine

		return 1;
	}

	int DisqueVirtuel::bd_mkdir(const std::string &p_DirName)
	{

		// Exemple avec "/doc/tmp/test"
		// path est égal à /doc/tmp
		// directory est égal à test
		// On devra ajouter le répertoire test à /doc/tmp

		int found = p_DirName.find_last_of("/");
		std::string chemin = p_DirName.substr(0, found);
		std::string repertoire = p_DirName.substr(found + 1);

		// Si le chemin d'accès est inexistant, on retourne 0
		if (chemin != "" && getBlock(chemin) == NULL)
		{
			std::cout << "Le répertoire n'existe pas" << std::endl;
			return 0;
		}

		// Si le répertoire existe déjà, on retourne 0
		if (getBlock(p_DirName) != NULL)
		{
			std::cout << "Le répertoire existe déjà" << std::endl;
			return 0;
		}

		// Sinon, on ajoute le répertoire au chemin
		ajouterRepertoireVide(getBlock(chemin), repertoire);

		return 1;
	}

	int DisqueVirtuel::bd_create(const std::string &p_FileName)
	{
		int found = p_FileName.find_last_of("/");
		std::string chemin = p_FileName.substr(0, found);
		std::string file = p_FileName.substr(found + 1);

		// Si le chemin d'accès est inexistant, on retourne 0
		if (chemin != "" && getBlock(chemin) == NULL)
		{
			std::cout << "Le répertoire n'existe pas" << std::endl;
			return 0;
		}

		// Si le répertoire existe déjà, on retourne 0
		if (getBlock(p_FileName) != NULL)
		{
			std::cout << "Le fichier existe déjà" << std::endl;
			return 0;
		}

		// Sinon, on ajoute le répertoire au chemin
		ajouterFichierVide(getBlock(chemin), file);

		return 1;
	}

	int DisqueVirtuel::bd_rm(const std::string &p_FileName)
	{
		Block *monBlock = getBlock(p_FileName);

		int found = p_FileName.find_last_of("/");
		std::string chemin = p_FileName.substr(0, found);
		std::string repertoire = p_FileName.substr(found + 1);

		// Si le chemin d'accès est inexistant, on retourne 0
		if (monBlock == NULL)
		{
			std::cout << "Ce chemin n'existe pas" << std::endl;
			return 0;
		}

		// Les métadonnées du répertoire parent sont mises à jour
		Block *parent = getBlock(chemin);

		// Répertoire
		if (monBlock->m_inode->st_mode == S_IFDIR)
		{

			// Si le répertoire n’est pas vide, ne faites rien et retournez 0.
			if (monBlock->m_dirEntry.size() > 2)
			{
				std::cout << "Ce repertoire n'est pas vide" << std::endl;
				return 0;
			}

			// Libérer l'inode si le nombre de liens est égal à 0
			if (monBlock->m_inode->st_nlink <= 2)
			{
				libererINode(monBlock->m_inode->st_ino);
			}
			libererBlock(monBlock->m_inode->st_block);

			parent->m_inode->st_nlink--;
		}

		// Fichier
		if (monBlock->m_inode->st_mode == S_IFREG)
		{
			// On libère l'inode
			libererINode(monBlock->m_inode->st_ino);
		}

		parent->m_inode->st_size -= 28;

		int i = 0;
		for (dirEntry *x : parent->m_dirEntry)
		{
			if (x->m_filename == repertoire)
				break;
			i++;
		}
		parent->m_dirEntry.erase(parent->m_dirEntry.begin() + i);
		return 1;
	}

	std::string DisqueVirtuel::bd_ls(const std::string &p_DirLocation)
	{
		Block *block = getBlock(p_DirLocation);

		if (block == NULL)
		{
			return "Le fichier n'existe pas.";
		}

		std::stringstream result;
		result << p_DirLocation << std::endl;

		for (int i = 0; i < block->m_dirEntry.size(); i++)
		{
			int inodeNumber = block->m_dirEntry[i]->m_iNode;
			iNode *inode = m_blockDisque[BASE_BLOCK_INODE + inodeNumber].m_inode;

			result << (inode->st_mode == S_IFDIR ? "d" : "-");
			result << std::setw(15) << std::right << block->m_dirEntry[i]->m_filename << " ";
			result << std::setw(8) << std::left << "Size: " << std::setw(5) << std::right << inode->st_size << " ";
			result << std::setw(10) << std::left << "inode: " << std::right << inodeNumber << " ";
			result << std::setw(10) << std::left << "nlink: " << std::right << inode->st_nlink << " ";
			result << std::endl;
		}
		return result.str();
	}

} // Fin du namespace