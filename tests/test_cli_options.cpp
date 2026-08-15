#include "doctest.h"

#include <cstdlib>
#include <string>
#include <vector>

#include "CliOptions.h"

namespace
{

/// @brief Construit un tableau argv modifiable a partir d'une liste de chaines.
class ArgumentVector
  {
  public:
    /// @brief Copie les arguments et prepare les pointeurs argv.
    /// @param p_arguments Arguments, argv[0] inclus.
    explicit ArgumentVector( const ::std::vector< ::std::string >& p_arguments )
      : m_storage( p_arguments ),
        m_pointers()
      {
      m_pointers.reserve( m_storage.size() );

      for ( ::std::string& argument : m_storage )
        {
        m_pointers.push_back( &argument[0] );
        }
      }

    /// @brief Nombre d'arguments.
    int count() const
      {
      const int argument_count = static_cast< int >( m_pointers.size() );
      return argument_count;
      }

    /// @brief Tableau argv.
    char** values()
      {
      return m_pointers.data();
      }

  private:
    ::std::vector< ::std::string > m_storage;  ///< Copies des arguments.
    ::std::vector< char* > m_pointers;         ///< Pointeurs vers les copies.
  };

} // namespace

TEST_CASE( "parse_arguments : aucun argument -> image par defaut" )
{
  ArgumentVector arguments( { "line_detector" } );
  CliOptions options;

  const int status = parse_arguments( arguments.count(), arguments.values(), options );

  CHECK( EXIT_SUCCESS == status );
  CHECK( SOURCE_KIND_IMAGE == options.source_kind );
  CHECK( DEFAULT_IMAGE_PATH == options.input_path );
}

TEST_CASE( "parse_arguments : --image avec chemin" )
{
  ArgumentVector arguments( { "line_detector", "--image", "img_piste/straight.jpg" } );
  CliOptions options;

  const int status = parse_arguments( arguments.count(), arguments.values(), options );

  CHECK( EXIT_SUCCESS == status );
  CHECK( SOURCE_KIND_IMAGE == options.source_kind );
  CHECK( ::std::string( "img_piste/straight.jpg" ) == options.input_path );
}

TEST_CASE( "parse_arguments : --video avec chemin" )
{
  ArgumentVector arguments( { "line_detector", "--video", "essai.avi" } );
  CliOptions options;

  const int status = parse_arguments( arguments.count(), arguments.values(), options );

  CHECK( EXIT_SUCCESS == status );
  CHECK( SOURCE_KIND_VIDEO == options.source_kind );
  CHECK( ::std::string( "essai.avi" ) == options.input_path );
}

TEST_CASE( "parse_arguments : --camera sans index -> index par defaut" )
{
  ArgumentVector arguments( { "line_detector", "--camera" } );
  CliOptions options;

  const int status = parse_arguments( arguments.count(), arguments.values(), options );

  CHECK( EXIT_SUCCESS == status );
  CHECK( SOURCE_KIND_CAMERA == options.source_kind );
  CHECK( DEFAULT_CAMERA_INDEX == options.camera_index );
}

TEST_CASE( "parse_arguments : --camera avec index explicite" )
{
  ArgumentVector arguments( { "line_detector", "--camera", "2" } );
  CliOptions options;

  const int status = parse_arguments( arguments.count(), arguments.values(), options );

  CHECK( EXIT_SUCCESS == status );
  CHECK( SOURCE_KIND_CAMERA == options.source_kind );
  CHECK( 2 == options.camera_index );
}

TEST_CASE( "parse_arguments : modes en conflit -> echec" )
{
  ArgumentVector arguments( { "line_detector", "--image", "a.jpg", "--video", "b.avi" } );
  CliOptions options;

  const int status = parse_arguments( arguments.count(), arguments.values(), options );

  CHECK( EXIT_FAILURE == status );
  CHECK( !options.error_message.empty() );
}

TEST_CASE( "parse_arguments : valeur manquante apres --image -> echec" )
{
  ArgumentVector arguments( { "line_detector", "--image" } );
  CliOptions options;

  const int status = parse_arguments( arguments.count(), arguments.values(), options );

  CHECK( EXIT_FAILURE == status );
  CHECK( !options.error_message.empty() );
}

TEST_CASE( "parse_arguments : flag inconnu -> echec" )
{
  ArgumentVector arguments( { "line_detector", "--webcam" } );
  CliOptions options;

  const int status = parse_arguments( arguments.count(), arguments.values(), options );

  CHECK( EXIT_FAILURE == status );
  CHECK( !options.error_message.empty() );
}

TEST_CASE( "parse_arguments : argument positionnel nu -> echec" )
{
  ArgumentVector arguments( { "line_detector", "img_piste/img2.jpg" } );
  CliOptions options;

  const int status = parse_arguments( arguments.count(), arguments.values(), options );

  CHECK( EXIT_FAILURE == status );
  CHECK( !options.error_message.empty() );
}

TEST_CASE( "parse_arguments : index camera non numerique -> echec" )
{
  ArgumentVector arguments( { "line_detector", "--camera", "abc" } );
  CliOptions options;

  const int status = parse_arguments( arguments.count(), arguments.values(), options );

  CHECK( EXIT_FAILURE == status );
  CHECK( !options.error_message.empty() );
}
