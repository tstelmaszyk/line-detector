#pragma once

/// @file
/// @brief Deux macros d'arrêt, pour deux causes distinctes (cf. CLAUDE.md,
/// section « Design by contract ») :
///  - `SMART_ASSERT` : violation d'invariant de code (bug interne, ne devrait
///    jamais arriver) -> `abort()`.
///  - `EXIT_IF_FAILED` : aléa d'environnement attendu (entrée utilisateur,
///    fichier, disque, périphérique) -> sortie propre `exit(EXIT_FAILURE)`.
/// Ne pas les confondre : la première signale un bug à corriger, la seconde
/// une situation normale à rapporter sans faire planter le programme.

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

// ============================================================================
// SMART_ASSERT — violation d'invariant de code.
//
// Ne devrait jamais se déclencher si le code est correct (Mat vide, mauvais
// nombre de canaux, homographie dégénérée, offset non fini...). Toujours
// actif, non supprimé par NDEBUG (contrairement à assert standard) : sur un
// véhicule, ces invariants doivent rester vérifiés même en release. Échec =
// message technique (fichier:ligne + condition) puis `abort()` : c'est un bug
// à corriger, pas une situation à gérer proprement.
// ============================================================================

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

#define SMART_ASSERT( p_cond, p_msg ) \
  do { if ( !( p_cond ) ) ::smart_assert::fail( __FILE__, __LINE__, #p_cond, ( p_msg ) ); } while ( 0 )

// ============================================================================
// EXIT_IF_FAILED — aléa d'environnement attendu.
//
// Pour une condition dont l'échec est une situation normale, pas un bug :
// arguments CLI invalides, source introuvable, disque plein, périphérique
// indisponible... Échec = message humain sur stderr (pas de fichier:ligne,
// pas de condition brute : ce message s'adresse à l'utilisateur, pas au
// développeur) puis `exit(EXIT_FAILURE)`, une sortie propre — pas `abort()`.
// Usage réservé à `main.cpp` : `line_detector_lib`/`line_detector_app` ne
// terminent jamais le process elles-mêmes (cf. CLAUDE.md, « Couche
// application »).
// ============================================================================

namespace smart_assert
{

/// @brief Affiche un message humain sur stderr et termine proprement le programme.
/// @param p_msg Message à afficher (destiné à l'utilisateur, pas au développeur).
[[noreturn]] inline void exit_with_message( const ::std::string& p_msg )
{
  ::std::cerr << p_msg << ::std::endl;
  ::std::exit( EXIT_FAILURE );
}

} // namespace smart_assert

#define EXIT_IF_FAILED( p_cond, p_msg ) \
  do { if ( !( p_cond ) ) ::smart_assert::exit_with_message( p_msg ); } while ( 0 )
