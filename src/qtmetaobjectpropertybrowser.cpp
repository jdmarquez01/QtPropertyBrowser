/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Solutions component.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include "qtmetaobjectpropertybrowser.h"
#include <QtVariantPropertyManager>
#include <qtpropertymanager.h>
#include <QMetaProperty>
#include <QStack>
#include <QMap>
#include <QDebug>


#include <QObject>
#if QT_VERSION >= 0x040400
QT_BEGIN_NAMESPACE
#endif

class QtMetaObjectPropertyBrowserPrivate
{


    Q_DECLARE_PUBLIC(QtMetaObjectPropertyBrowser)
public:

    QtMetaObjectPropertyBrowserPrivate(QtMetaObjectPropertyBrowser *q);
    QtVariantPropertyManager	*var;
    QtVariantEditorFactory *variantFactory;

    QMap<QtProperty*,QMetaProperty> m_propertyMap;
    QMap<QtProperty*, const QObject*> m_propertyObject;
    const QObject* m_currentObject;


    public Q_SLOTS:
    void slotValueChanged(QtProperty*p, const QVariant &val);

    QtMetaObjectPropertyBrowser* q_ptr;


    QtVariantProperty *RecursiveCreateProperty(const QMetaProperty &p , QVariant &value, const QString &name);

    QtVariantProperty *CreateEnumFlagProperty(const QMetaEnum &e, const QVariant &value, const QString &name);
    QtVariantProperty *CreateSupportedMetaProperty(const QVariant &value, const QString &name);


};





QtMetaObjectPropertyBrowserPrivate::QtMetaObjectPropertyBrowserPrivate(QtMetaObjectPropertyBrowser *q) : q_ptr(q), var(new QtVariantPropertyManager(q))
{
   QObject::connect(var,SIGNAL(valueChanged(QtProperty*,QVariant)),q,SLOT(slotValueChanged(QtProperty*,QVariant)));
  //QObject::connect(var, SIGNAL(valueChanged(QtProperty*,const QVariant&)),this,SLOT(slotValueChanged(QtProperty*,const QVariant&)) );

    m_currentObject = 0;
}

/**
 * @brief QtMetaObjectPropertyBrowserPrivate::CreateEnumFlagProperty
 * @param e
 * @param value
 * @param name
 * @return
 */
QtVariantProperty *QtMetaObjectPropertyBrowserPrivate::CreateEnumFlagProperty(const QMetaEnum &e, const QVariant &value, const QString &name)
{

	QStringList sl;
	QMap<int, int> val;
	QtVariantProperty* property = var->addProperty(e.isFlag() ? var->flagTypeId() : var->enumTypeId(), name);
	for (int j = 0; j < e.keyCount(); j++)
	{
		sl << e.key(j);

		if (!e.isFlag())
			val.insert(e.value(j), j);
		else
			val.insert(j, e.value(j));
	}
	QString s = e.isFlag() ? QString::fromAscii("flagNames") : QString::fromAscii("enumNames");

	property->setAttribute(s, sl);
	QVariant v;
	v.setValue(val);
	property->setAttribute(e.isFlag() ? QString::fromAscii("flagValues") : QLatin1String("enumValues"), v);

	var->setValue(property, value);
	return property;
}

/**
 * @brief QtMetaObjectPropertyBrowserPrivate::CreateSupportedMetaProperty
 * @param p
 * @param value
 * @param name
 * @return
 */
QtVariantProperty *QtMetaObjectPropertyBrowserPrivate::CreateSupportedMetaProperty(const QVariant &value, const QString &name)
{

	QtVariantProperty *property = var->addProperty(value.type(), name);

	if (property)
	{
		//        int ll = -1;
		//        int ul = -1;
		//        switch (value.type()) {
		//            case QMetaType::UChar: ll = 0; ul = UCHAR_MAX; break;
		//            case QMetaType::Char: ll = CHAR_MIN; ul =CHAR_MAX; break;
		//            case QMetaType::Short: ll = SHRT_MIN; ul = SHRT_MAX; break;
		//            case QMetaType::UShort: ll = 0 ; ul = USHRT_MAX; break;
		//            case QMetaType::UInt: ll = 0 ; ul = UINT_MAX; break;
		//            case QMetaType::Int: ll = INT_MIN; ul = INT_MAX; break;
		//        default:
		//            break;
		//        }
		//        if (ll != ul)
		//        {
		//            var->setAttribute(property,"minimun",ll);
		//            var->setAttribute(property,"maximum",ul);
		//        }

		var->setValue(property, value);
	}
	return property;
}



/**
  * @brief QtMetaObjectPropertyBrowserPrivate::RecursiveCreateProperty
  * @param parent padre;
  * @param value valor de la propiedad a crear
  * @param cadena de texto
  * @return referencia a la property creada o NULL si no hay suerte
  */

QtVariantProperty *QtMetaObjectPropertyBrowserPrivate::RecursiveCreateProperty(const QMetaProperty &p, QVariant &value, const QString &name)
{

	if (!value.isValid())
		return 0;

	QtVariantProperty *prop = 0;

	if (value.canConvert(QMetaType::QVariantList))
	{
		QVariantList vl = qvariant_cast<QVariantList>(value);
		prop = var->addProperty(var->groupTypeId(), QString("%1[]").arg(name));
		for (int i = 0; i < vl.size(); ++i)
		{
			prop->addSubProperty(RecursiveCreateProperty(p, vl[i], QString("[%1]").arg(i)));
		}

	}
	else if (value.canConvert(QMetaType::QVariantMap))
	{
		QVariantMap vl = qvariant_cast<QVariantMap>(value);
		prop = var->addProperty(var->groupTypeId(), QString("%1[]").arg(name));
		
		for (QVariantMap::const_iterator iter = vl.begin(); iter != vl.end(); ++iter) {
		  prop->addSubProperty(RecursiveCreateProperty(p, const_cast<QVariant&>(iter.value()), QString("[%1]").arg(iter.key())));
		}


	}

   else if(value.canConvert(QMetaType::QObjectStar))
   {

      prop = var->addProperty(var->groupTypeId(),name);

      QObject *object = qvariant_cast<QObject*>(value);
      if (object == 0)
           return prop;

      const QMetaObject *meta = object->metaObject();
      QtVariantProperty *property = 0;

      for (int i = 1 ; i < meta->propertyCount(); ++i)
      {
          QMetaProperty   metaProperty = meta->property(i);
          QVariant        value = metaProperty.read(object);

          if (metaProperty.isEnumType())  //ENUMERAO
          {
             property = this->CreateEnumFlagProperty(metaProperty.enumerator(),value,metaProperty.name());
          }
          else if (var->isPropertyTypeSupported(metaProperty.type())) //TIPO BASICO
          {
              property = CreateSupportedMetaProperty(value,metaProperty.name());
          }
          else
          {
                property = RecursiveCreateProperty(metaProperty,value,metaProperty.name());
          }
          if (property != 0)
          {
            prop->addSubProperty(property);
            m_propertyMap[property] = metaProperty;
            m_propertyObject[property] = object;

          }
      }

   }
   else
   {
      prop = CreateSupportedMetaProperty(value,name);
   }

   return prop;
 }



void QtMetaObjectPropertyBrowserPrivate::slotValueChanged(QtProperty *p, const QVariant &val)
{
  QMap<QtProperty*,QMetaProperty>::iterator it =m_propertyMap.find(p);
  if (it != m_propertyMap.end())
  {
    QMetaProperty mProp = *it;

    const QObject *ob = m_propertyObject[p];
    if (ob != 0)
    {
        QVariant v = mProp.read(ob);
        if ( v != val)
        {
          mProp.write(const_cast<QObject*>(ob),val);
          //Reload

          QtVariantProperty *vp = dynamic_cast<QtVariantProperty*>(p);

          if (vp != NULL)
          {
            v = mProp.read(ob);
            if (v!=val)
            {
              vp->setValue(v);

            }
        }
        }
    }
  }

}


QtMetaObjectPropertyBrowser::QtMetaObjectPropertyBrowser(QWidget *parent) : QtTreePropertyBrowser(parent), d_ptr(new QtMetaObjectPropertyBrowserPrivate(this))
{

    //d_ptr->var = new QtVariantPropertyManager;

    d_ptr->variantFactory  = new QtVariantEditorFactory(this);
    this->setFactoryForManager(d_ptr->var,d_ptr->variantFactory);

    this->setPropertiesWithoutValueMarked(true);
    //this->setRootIsDecorated(false);
    this->setHeaderVisible(false);




}
QtMetaObjectPropertyBrowser::~QtMetaObjectPropertyBrowser()
{

    delete d_ptr->variantFactory;
}






void QtMetaObjectPropertyBrowser::SetObject(const QObject *object)
{
    if (d_ptr->m_currentObject == object)
        return;

	const QObject *temp = object;

    this->clear();
    d_ptr->m_propertyMap.clear();
    d_ptr->m_propertyObject.clear();

    this->setUpdatesEnabled(false);
    if (object)
    {
        d_ptr->m_currentObject = object;
		const QMetaObject *meta = object->metaObject();

        QStack<const QMetaObject*> inheritanceStack;

        while (meta!= &QObject::staticMetaObject)
        {
            inheritanceStack.push(meta);
			
			try
			{
				if (meta)
				meta = meta->superClass();
			}
			catch (...)
			{
				meta = NULL;
			}
        }

        QtVariantProperty *property ,*gproperty;

        while (inheritanceStack.size() != 0)
        {
            meta = inheritanceStack.pop();
            if (meta->propertyOffset() == meta->propertyCount())
              continue;
            gproperty =d_ptr->var->addProperty(d_ptr->var->groupTypeId(),meta->className());
            this->addProperty(gproperty);

            for (int i = meta->propertyOffset(); i < meta->propertyCount(); i++)
            {
                QMetaProperty   metaProperty = meta->property(i);
                QVariant        value = metaProperty.read(object);
                property = NULL;

                if (metaProperty.isEnumType())  //ENUMERAO
                    property = d_ptr->CreateEnumFlagProperty(metaProperty.enumerator(),value,metaProperty.name());
                else if (d_ptr->var->isPropertyTypeSupported(metaProperty.type())) //TIPO BASICO
                    property = d_ptr->CreateSupportedMetaProperty(value,metaProperty.name());
                else
                    property = d_ptr->RecursiveCreateProperty(metaProperty,value,metaProperty.name());


                if (property)
                {
                    gproperty->addSubProperty(property);

                   d_ptr->m_propertyMap[property] = metaProperty;
                   d_ptr->m_propertyObject[property] = object;

                   this->addProperty(gproperty);

                }
                else
                {
                    qDebug() << "Tipo no detectado: "  << metaProperty.name();
                }


            }
			


        } //dynamic_properties
		
		
		//dynamic_properties
		gproperty = d_ptr->var->addProperty(d_ptr->var->groupTypeId(), "dynamic");
		this->addProperty(gproperty);
		QList<QByteArray> propNames = object->dynamicPropertyNames();
        for (QList<QByteArray>::iterator it = propNames.begin(); it < propNames.end(); ++it)
		{

			//QMetaProperty   metaProperty = meta->property(i);
			const char * property_name = it->constData();
			QVariant        value = object->property(property_name);
			property = NULL;
			property = d_ptr->CreateSupportedMetaProperty(value, property_name);


			if (property)
			{
				gproperty->addSubProperty(property);

				//d_ptr->m_propertyMap[property] = it->data();
				d_ptr->m_propertyObject[property] = object;

				this->addProperty(gproperty);

			}
			else
			{
				qDebug() << "Tipo no detectado: " << value.type();
			}



		}


         this->setPropertiesWithoutValueMarked(true);
    }
    this->setUpdatesEnabled(true);



}






#include "moc_qtmetaobjectpropertybrowser.cpp"


#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif
