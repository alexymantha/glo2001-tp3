/**
 * \file block.cpp
 * \brief Implémentation d'un bloc.
 * \author ?
 * \version 0.1
 * \date  2022
 *
 *  Travail pratique numéro 3
 *
 */

#include "block.h"
// vous pouvez inclure d'autres librairies si c'est nécessaire

namespace TP3
{
	Block::Block() {}
	Block::Block(size_t td) : m_type_donnees(td) {}
	Block::~Block() {}
}

// Fin du namespace
