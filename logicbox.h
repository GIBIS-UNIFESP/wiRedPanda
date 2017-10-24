#ifndef LOGICBOX_H
#define LOGICBOX_H

#include <QFileSystemWatcher>
#include <logicelement.h>

// TODO:

// Pode retornar quais são os inputs, e outputs,
// e também sabe quem está usando esse protótipo.

// Não só é utilizado para gerar circuitos de "baixo nivel",
// como também gerencia as boxes gráficas, armazenando previamente
// e linkando as entradas com as saídas.

// Em algum lugar posso tentar fazer associação entre o visual e a
// o circuito respectivo, talvez uma terceira classe só pra isso.

//Mudar logicbox para algum nome que faça mais sentido.

class LogicBox : public LogicElement {
  QString filename;

public:
  LogicBox( QString fname );
  virtual ~LogicBox( );
  static LogicBox* loadFile( QString fname );

  void reload( );

  QString getFilename( ) const;

  QFileSystemWatcher watcher;
protected:
  void _updateLogic( const std::vector< bool > &inputs );
};

#endif // LOGICBOX_H
