#include "boxwatcher.h"

BoxWatcher::BoxWatcher( const QString &filename ) : m_filename( filename ) {
  m_fileWatcher.addPath( filename );
}

void BoxWatcher::fileUpdated( const QString & ) {

}
