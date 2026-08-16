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

TEST_CASE( "parse_arguments : --image suivi d'un flag -> valeur manquante" )
{
  ArgumentVector arguments( { "line_detector", "--image", "--video", "b.avi" } );
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

TEST_CASE( "parse_arguments : index camera hors plage -> echec" )
{
  ArgumentVector arguments( { "line_detector", "--camera", "99999999999999999999" } );
  CliOptions options;

  const int status = parse_arguments( arguments.count(), arguments.values(), options );

  CHECK( EXIT_FAILURE == status );
  CHECK( !options.error_message.empty() );
}

TEST_CASE( "parse_arguments : --record absent par defaut" )
{
  ArgumentVector arguments( { "line_detector", "--image", "a.jpg" } );
  CliOptions options;

  const int status = parse_arguments( arguments.count(), arguments.values(), options );

  CHECK( EXIT_SUCCESS == status );
  CHECK( false == options.record );
}

TEST_CASE( "parse_arguments : --record avec chaque mode" )
{
  ArgumentVector image_arguments( { "line_detector", "--image", "a.jpg", "--record" } );
  CliOptions image_options;
  const int image_status = parse_arguments( image_arguments.count(), image_arguments.values(), image_options );

  ArgumentVector video_arguments( { "line_detector", "--record", "--video", "b.avi" } );
  CliOptions video_options;
  const int video_status = parse_arguments( video_arguments.count(), video_arguments.values(), video_options );

  ArgumentVector camera_arguments( { "line_detector", "--camera", "1", "--record" } );
  CliOptions camera_options;
  const int camera_status = parse_arguments( camera_arguments.count(), camera_arguments.values(), camera_options );

  CHECK( EXIT_SUCCESS == image_status );
  CHECK( true == image_options.record );
  CHECK( EXIT_SUCCESS == video_status );
  CHECK( true == video_options.record );
  CHECK( SOURCE_KIND_VIDEO == video_options.source_kind );
  CHECK( EXIT_SUCCESS == camera_status );
  CHECK( true == camera_options.record );
  CHECK( 1 == camera_options.camera_index );
}
