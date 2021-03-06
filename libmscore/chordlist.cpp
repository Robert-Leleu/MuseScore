//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "config.h"
#include "chordlist.h"
#include "xml.h"
#include "pitchspelling.h"
#include "mscore.h"

//---------------------------------------------------------
//   HChord
//---------------------------------------------------------

HChord::HChord(const QString& str)
      {
      static const char* const scaleNames[2][12] = {
            { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" },
            { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" }
            };
      keys = 0;
      QStringList sl = str.split(" ", QString::SkipEmptyParts);
      foreach(const QString& s, sl) {
            for (int i = 0; i < 12; ++i) {
                  if (s == scaleNames[0][i] || s == scaleNames[1][i]) {
                        operator+=(i);
                        break;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   HChord
//---------------------------------------------------------

HChord::HChord(int a, int b, int c, int d, int e, int f, int g, int h, int i, int k, int l)
      {
      keys = 0;
      if (a >= 0)
            operator+=(a);
      if (b >= 0)
            operator+=(b);
      if (c >= 0)
            operator+=(c);
      if (d >= 0)
            operator+=(d);
      if (e >= 0)
            operator+=(e);
      if (f >= 0)
            operator+=(f);
      if (g >= 0)
            operator+=(g);
      if (h >= 0)
            operator+=(h);
      if (i >= 0)
            operator+=(i);
      if (k >= 0)
            operator+=(k);
      if (l >= 0)
            operator+=(l);
      }

//---------------------------------------------------------
//   rotate
//    rotate 12 Bits
//---------------------------------------------------------

void HChord::rotate(int semiTones)
      {
      while (semiTones > 0) {
            if (keys & 0x800)
                  keys = ((keys & ~0x800) << 1) + 1;
            else
                  keys <<= 1;
            --semiTones;
            }
      while (semiTones < 0) {
            if (keys & 1)
                  keys = (keys >> 1) | 0x800;
            else
                  keys >>= 1;
            ++semiTones;
            }
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

QString HChord::name(int tpc)
      {
      static const HChord C0(0,3,6,9);
      static const HChord C1(0,3);

      QString buf = tpc2name(tpc, false);
      HChord c(*this);

      int key = tpc2pitch(tpc);

      c.rotate(-key);        // transpose to C

      // special cases
      if (c == C0) {
            buf += "dim";
            return buf;
            }
      if (c == C1) {
            buf += "no5";
            return buf;
            }

      bool seven   = false;
      bool sharp9  = false;
      bool nat11   = false;
      bool sharp11 = false;
      bool nat13   = false;
      bool flat13  = false;

      // minor?
      if (c.contains(3)) {
            if (!c.contains(4))
                  buf += "m";
            else
                  sharp9 = true;
            }

      // 7
      if (c.contains(11)) {
            buf += "Maj7";
            seven = true;
            }
      else if (c.contains(10)) {
            buf += "7";
            seven = true;
            }

      // 4
      if (c.contains(5)) {
            if (!c.contains(4)) {
                  buf += "sus4";
                  }
            else
                  nat11 = true;
            }

      // 5
      if (c.contains(7)) {
            if (c.contains(6))
                  sharp11 = true;
            if (c.contains(8))
                  flat13 = true;
            }
      else {
            if (c.contains(6))
                  buf += "b5";
            if (c.contains(8))
                  buf += "#5";
            }

      // 6
      if (c.contains(9)) {
            if (!seven)
                  buf += "6";
            else
                  nat13 = true;
            }

      // 9
      if (c.contains(1))
            buf += "b9";
      if (c.contains(2))
            buf += "9";
      if (sharp9)
            buf += "#9";

      // 11
      if (nat11)
            buf += "11 ";
      if (sharp11)
            buf += "#11";

      // 13
      if (flat13)
            buf += "b13";
      if (nat13) {
            if (c.contains(1) || c.contains(2) || sharp9 || nat11 || sharp11)
                  buf += "13";
            else
                  buf += "add13";
            }
      return buf;
      }

//---------------------------------------------------------
//   print
//---------------------------------------------------------

void HChord::print() const
      {
      const char* names[] = { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" };

      for (int i = 0; i < 12; i++) {
            if (contains(i))
                  qDebug(" %s", names[i]);
            }
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void HChord::add(const QList<HDegree>& degreeList)
      {
// qDebug("HChord::add   ");print();
      // convert degrees to semitones
      static const int degreeTable[] = {
            // 1  2  3  4  5  6   7
            // C  D  E  F  G  A   B
               0, 2, 4, 5, 7, 9, 11
            };
      // factor in the degrees
      foreach(const HDegree& d, degreeList) {
            int dv  = degreeTable[(d.value() - 1) % 7] + d.alter();
            int dv1 = degreeTable[(d.value() - 1) % 7];

            if (d.value() == 7 && d.alter() == 0) {
                  // DEBUG: seventh degree is Bb, not B
                  //        except Maj   (TODO)
                  dv -= 1;
                  }

            if (d.type() == ADD)
                  *this += dv;
            else if (d.type() == ALTER) {
                  if (contains(dv1)) {
                        *this -= dv1;
                        *this += dv;
                        }
                  else {
//                        qDebug("ALTER: chord does not contain degree %d(%d):",
//                           d.value(), d.alter());
//                        print();
                        *this += dv;      // DEBUG: default to add
                        }
                  }
            else if (d.type() == SUBTRACT) {
                  if (contains(dv1))
                        *this -= dv1;
                  else {
                        qDebug("SUB: chord does not contain degree %d(%d):",
                           d.value(), d.alter());
                        print();
                        }
                  }
            else
                  qDebug("degree type %d not supported", d.type());

// qDebug("  HCHord::added  "); print();
            }
      }

//---------------------------------------------------------
//   readRenderList
//---------------------------------------------------------

static void readRenderList(QString val, QList<RenderAction>& renderList)
      {
      QStringList sl = val.split(" ", QString::SkipEmptyParts);
      foreach(const QString& s, sl) {
            if (s.startsWith("m:")) {
                  QStringList ssl = s.split(":", QString::SkipEmptyParts);
                  if (ssl.size() == 3) {
                        RenderAction a;
                        a.type = RenderAction::RENDER_MOVE;
                        a.movex = ssl[1].toDouble();
                        a.movey = ssl[2].toDouble();
                        renderList.append(a);
                        }
                  }
            else if (s == ":push")
                  renderList.append(RenderAction(RenderAction::RENDER_PUSH));
            else if (s == ":pop")
                  renderList.append(RenderAction(RenderAction::RENDER_POP));
            else if (s == ":n")
                  renderList.append(RenderAction(RenderAction::RENDER_NOTE));
            else if (s == ":a")
                  renderList.append(RenderAction(RenderAction::RENDER_ACCIDENTAL));
            else {
                  RenderAction a(RenderAction::RENDER_SET);
                  a.text = s;
                  renderList.append(a);
                  }
            }
      }

//---------------------------------------------------------
//   writeRenderList
//---------------------------------------------------------

static void writeRenderList(Xml& xml, const QList<RenderAction>* al, const QString& name)
      {
      QString s;

      int n = al->size();
      for (int i = 0; i < n; ++i) {
            if (!s.isEmpty())
                  s += " ";
            const RenderAction& a = (*al)[i];
            switch(a.type) {
                  case RenderAction::RENDER_SET:
                        s += a.text;
                        break;
                  case RenderAction::RENDER_MOVE:
                        if (a.movex != 0.0 || a.movey != 0.0)
                              s += QString("m:%1:%2").arg(a.movex).arg(a.movey);
                        break;
                  case RenderAction::RENDER_PUSH:
                        s += ":push";
                        break;
                  case RenderAction::RENDER_POP:
                        s += ":pop";
                        break;
                  case RenderAction::RENDER_NOTE:
                        s += ":n";
                        break;
                  case RenderAction::RENDER_ACCIDENTAL:
                        s += ":a";
                        break;
                  }
            }
      xml.tag(name, s);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void ChordDescription::read(const QDomElement& de)
      {
      id = de.attribute("id").toInt();
      for (QDomElement e = de.firstChildElement(); !e.isNull();  e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if (tag == "name")
                  names.append(val);
            else if (tag == "xml")
                  xmlKind = val;
            else if (tag == "degree")
                  xmlDegrees.append(val);
            else if (tag == "voicing")
                  chord = HChord(val);
            else if (tag == "render")
                  readRenderList(val, renderList);
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void ChordDescription::write(Xml& xml)
      {
      xml.stag(QString("chord id=\"%1\"").arg(id));
      foreach(const QString& s, names)
            xml.tag("name", s);
      xml.tag("xml", xmlKind);
      xml.tag("voicing", chord.getKeys());
      foreach(const QString& s, xmlDegrees)
            xml.tag("degree", s);
      writeRenderList(xml, &renderList, "render");
      xml.etag();
      }

//---------------------------------------------------------
//   ~ChordList
//---------------------------------------------------------

ChordList::~ChordList()
      {
      if (isDetached()) {
            QMapIterator<int, ChordDescription*> i(*this);
            while(i.hasNext()) {
                  i.next();
                  delete i.value();
                  }
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void ChordList::read(const QDomElement& de)
      {
      int fontIdx = 0;
      for (QDomElement e = de.firstChildElement(); !e.isNull();  e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if (tag == "font") {
                  ChordFont f;
                  f.family = e.attribute("family", "default");
                  f.mag    = 1.0;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull();  ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "sym") {
                              ChordSymbol cs;
                              cs.fontIdx = fontIdx;
                              cs.name    = ee.attribute("name");
                              cs.code    = ee.attribute("code").toInt(0, 0);
                              symbols.insert(cs.name, cs);
                              }
                        else if (ee.tagName() == "mag") {
                              f.mag = ee.text().toDouble();
                              }
                        else
                              domError(ee);
                        }
                  fonts.append(f);
                  ++fontIdx;
                  }
            else if (tag == "chord") {
                  int id = e.attribute("id").toInt();
                  ChordDescription* cd = take(id);
                  if (cd == 0)
                        cd = new ChordDescription();
                  cd->read(e);
                  insert(id, cd);
                  }
            else if (tag == "renderRoot")
                  readRenderList(val, renderListRoot);
            else if (tag == "renderBase")
                  readRenderList(val, renderListBase);
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void ChordList::write(Xml& xml)
      {
      int fontIdx = 0;
      foreach (ChordFont f, fonts) {
            xml.stag(QString("font id=\"%1\" family=\"%2\"").arg(fontIdx).arg(f.family));
            xml.tag("mag", f.mag);
            foreach(ChordSymbol s, symbols) {
                  if (s.fontIdx == fontIdx) {
                        xml.tagE(QString("sym name=\"%1\" code=\"%2\"").arg(s.name).arg(s.code.unicode()));
                        }
                  }
            xml.etag();
            ++fontIdx;
            }
      if (!renderListRoot.isEmpty())
            writeRenderList(xml, &renderListRoot, "renderRoot");
      if (!renderListBase.isEmpty())
            writeRenderList(xml, &renderListBase, "renderBase");
      foreach(ChordDescription* d, *this)
            d->write(xml);
      }

//---------------------------------------------------------
//   read
//    read Chord List, return false on error
//---------------------------------------------------------

bool ChordList::read(const QString& name)
      {
      QString path;
      QFileInfo ftest(name);
      if (ftest.isAbsolute())
            path = name;
      else {
#ifdef Q_WS_IOS
            path = QString("%1/%2").arg(MScore::globalShare()).arg(name);
#else
            path = QString("%1styles/%2").arg(MScore::globalShare()).arg(name);
#endif
            }
      //default to stdchords.xml
      QFileInfo fi(path);
      if (!fi.exists())
#ifdef Q_WS_IOS
            path = QString("%1/%2").arg(MScore::globalShare()).arg("stdchords.xml");
#else
            path = QString("%1styles/%2").arg(MScore::globalShare()).arg("stdchords.xml");
#endif

      if (name.isEmpty())
            return false;
      QFile f(path);
      if (!f.open(QIODevice::ReadOnly)) {
            QString s = QT_TRANSLATE_NOOP("file", "cannot open chord description:\n%1\n%2");
            MScore::lastError = s.arg(f.fileName()).arg(f.errorString());
qDebug("ChordList::read failed: <%s>", qPrintable(path));
            return false;
            }
      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(&f, false, &err, &line, &column)) {
            QString s = QT_TRANSLATE_NOOP("file", "error reading chord description %1 at line %2 column %3: %4\n");
            MScore::lastError = s.arg(f.fileName()).arg(line).arg(column).arg(err);
            return false;
            }
      docName = f.fileName();

      for (QDomElement e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "museScore") {
                  // QString version = e.attribute(QString("version"));
                  // QStringList sl = version.split('.');
                  // int _mscVersion = sl[0].toInt() * 100 + sl[1].toInt();
                  read(e);
                  return true;
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   writeChordList
//---------------------------------------------------------

bool ChordList::write(const QString& name)
      {
      QFileInfo info(name);

      if (info.suffix().isEmpty()) {
            QString path = info.filePath();
            path += QString(".xml");
            info.setFile(path);
            }

      QFile f(info.filePath());

      if (!f.open(QIODevice::WriteOnly)) {
            QString s = QT_TRANSLATE_NOOP("file", "Open Chord Description\n%1\nfailed: %2");
            MScore::lastError = s.arg(f.fileName()).arg(f.errorString());
            return false;
            }

      Xml xml(&f);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");

      write(xml);
      xml.etag();
      if (f.error() != QFile::NoError) {
            QString s = QT_TRANSLATE_NOOP("file", "Write Chord Description failed: %1");
            MScore::lastError = s.arg(f.errorString());
            }
      return true;
      }




