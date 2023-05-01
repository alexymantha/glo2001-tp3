/**
 * \file block.h
 * \brief Gestion d'un bloc dans un disque virtuel.
 * \author GLO-2001
 * \version 0.1
 * \date 2022
 *
 *  Travail pratique numéro 3
 *
 */

#include <string>
#include <vector>

#ifndef _BLOCK__H
#define _BLOCK__H

namespace TP3
{

// Les flags suivants sont pour st_mode
#define S_IFREG  0010   // Indique que l'i-node est un fichier ordinaire
#define S_IFDIR  0020   // Indique que l'i-node est un repertoire
// Les flags suivants sont pour identifier le type de données stockées dans le bloc
#define S_IFBL 	0010	// indique que le bloc contient le bitmap des blocs libres
#define S_IFIL 	0020	// indique que le bloc contient le bitmap des inodes libres
#define S_IFIN 	0030	// indique que le bloc contient des métadonnées (donc un inode)
#define S_IFDE 	0040	// indique que le bloc contient la liste des dirEntry (informations d'un répertoire)

/**
 * \struct iNode
 * \brief Une struture utilisée pour stocker les métadonnées
 */
struct iNode
{
	size_t st_ino;		// numero de l'i-node
	size_t st_mode;		// S_IFREG ou S_IFDIR. Normalement contient aussi RWX
	size_t st_nlink;	// nombre de lien pointant vers l'i-node
	size_t st_size;		// taille du fichier, en octets
	size_t st_block;	// numero du block (un seul bloc dans le TP pour simplifier)

	iNode(size_t i, size_t m, size_t n, size_t s, size_t b) : st_ino(i), st_mode(m), st_nlink(n), st_size(s), st_block(b) {}
};

/**
 * \struct dirEntry
 * \brief Une struture utilisée pour stocker les données d'un répertoire
 */
struct dirEntry
{
	size_t m_iNode;   		// numero de l'i-node
	std::string m_filename;	// nom du fichier ou du répertoire

	dirEntry(size_t i, std::string m) : m_iNode(i), m_filename(m) {}
};

/**
 *
 */
class Block {
public:
	Block();			// Constructeur par défaut
    Block(size_t td);	// Constructeur pour initialiser m_type_donnees
    ~Block();			// Destructeur

private:
	// Il est interdit de modifier ce modèle d'implémentation (i.e. les types des membres privés)!
	size_t m_type_donnees; 		// peut être S_IFIL, S_IFBL, S_IFIN ou S_IFDE
    std::vector<bool> m_bitmap;	// pour stocker la liste des blocks libres ou les inodes libres
    iNode * m_inode;			// pour stocker les métadonnées d'un fichier ou d'un répertoire
    std::vector<dirEntry*> m_dirEntry;	// pour stocker la liste des dirEntry (les informations d'un répertoire)
    friend class DisqueVirtuel;	// La classe DisqueVirtuel est amie pour avoir accès à la partie privée !
};

}//Fin du namespace

#endif
