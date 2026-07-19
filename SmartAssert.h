#pragma once

/// @file
/// @brief Assertion toujours active (non supprimée par NDEBUG) pour les
/// invariants critiques du pipeline.

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

namespace smart_assert
{

/// @brief Formate un message d'assertion lisible.
/// @param p_file Fichier source (__FILE__).
/// @param p_line Ligne source (__LINE__).
/// @param p_cond Texte de la condition violée.
/// @param p_msg  Message additionnel (peut être vide).
/// @return Chaîne formatée décrivant l'échec.
inline ::std::string format( const char* p_file,
                             int p_line,
                             const char* p_cond,
                             const ::std::string& p_msg )
{
  ::std::ostringstream oss;
  oss << "SMART_ASSERT echoue: (" << p_cond << ") a " << p_file << ":" << p_line;

  if ( !p_msg.empty() )
    {
    oss << " -- " << p_msg;
    }

  return oss.str();
}

/// @brief Affiche le message d'échec et interrompt le programme.
/// @param p_file Fichier source (__FILE__).
/// @param p_line Ligne source (__LINE__).
/// @param p_cond Texte de la condition violée.
/// @param p_msg  Message additionnel.
[[noreturn]] inline void fail( const char* p_file,
                               int p_line,
                               const char* p_cond,
                               const ::std::string& p_msg )
{
  ::std::cerr << format( p_file, p_line, p_cond, p_msg ) << ::std::endl;
  ::std::abort();
}

} // namespace smart_assert

// Toujours actif : NON supprime par NDEBUG (contrairement a assert). Pour un
// vehicule, les invariants critiques doivent rester actifs meme en release.
#define SMART_ASSERT( p_cond, p_msg ) \
  do { if ( !( p_cond ) ) ::smart_assert::fail( __FILE__, __LINE__, #p_cond, ( p_msg ) ); } while ( 0 )
