/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2011 hms <<user@hostname.org>>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-mediastore
 *
 * FIXME:Describe mediastore here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! mediastore ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <stdlib.h>
#include <sqlite3.h>

#include "gstmediastore.h"

GST_DEBUG_CATEGORY_STATIC (gst_media_store_debug);
#define GST_CAT_DEFAULT gst_media_store_debug

//Enable to print debug messages
//#define DEBUG

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SILENT
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

/*GST_BOILERPLATE (GstMediaStore, gst_media_store, GstElement,
    GST_TYPE_ELEMENT);*/
GST_BOILERPLATE (GstMediaStore, gst_media_store, GstBaseSink,
    GST_TYPE_BASE_SINK);

static void gst_media_store_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_media_store_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_media_store_set_caps (GstPad * pad, GstCaps * caps);
//static GstFlowReturn gst_media_store_chain (GstPad * pad, GstBuffer * buf);
static GstFlowReturn gst_media_store_render (GstBaseSink * sink,
    GstBuffer * buffer);

/* GObject vmethod implementations */

static void
gst_media_store_base_init (gpointer gclass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (gclass);

  gst_element_class_set_details_simple(element_class,
    "mediastore",
    "Sink/Database",
    "Database operations",
    "Vinay <<user@hostname.org>>");

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&sink_factory));
}

/* initialize the mediastore's class */
static void
gst_media_store_class_init (GstMediaStoreClass * klass)
{
  GObjectClass *gobject_class;
  GstBaseSinkClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = GST_BASE_SINK_CLASS (klass);

  gobject_class->set_property = gst_media_store_set_property;
  gobject_class->get_property = gst_media_store_get_property;

  g_object_class_install_property (gobject_class, PROP_SILENT,
      g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
          FALSE, G_PARAM_READWRITE));

  gstelement_class->render = GST_DEBUG_FUNCPTR (gst_media_store_render);
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_media_store_init (GstMediaStore * mediastore,
    GstMediaStoreClass * gclass)
{
  //mediastore->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
  //gst_pad_set_setcaps_function (mediastore->sinkpad, GST_DEBUG_FUNCPTR(gst_media_store_set_caps));
  //gst_pad_set_getcaps_function (mediastore->sinkpad, GST_DEBUG_FUNCPTR(gst_pad_proxy_getcaps));
  /*gst_pad_set_chain_function (mediastore->sinkpad,
                              GST_DEBUG_FUNCPTR(gst_media_store_chain));*/

  mediastore->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
  gst_pad_set_getcaps_function (mediastore->srcpad,
                                GST_DEBUG_FUNCPTR(gst_pad_proxy_getcaps));

  //gst_element_add_pad (GST_ELEMENT (mediastore), mediastore->sinkpad);
  gst_element_add_pad (GST_ELEMENT (mediastore), mediastore->srcpad);
  mediastore->silent = FALSE;
}

static void
gst_media_store_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstMediaStore *mediastore = GST_MEDIASTORE (object);

  switch (prop_id) {
    case PROP_SILENT:
      mediastore->silent = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_media_store_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstMediaStore *mediastore = GST_MEDIASTORE (object);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean (value, mediastore->silent);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstElement vmethod implementations */

/* this function handles the link with other elements */
static gboolean
gst_media_store_set_caps (GstPad * pad, GstCaps * caps)
{
  GstMediaStore *mediastore;
  GstPad *otherpad;

  mediastore = GST_MEDIASTORE (gst_pad_get_parent (pad));
  otherpad = (pad == mediastore->srcpad) ? mediastore->sinkpad : mediastore->srcpad;
  gst_object_unref (mediastore);

  return gst_pad_set_caps (otherpad, caps);
}


static GstFlowReturn
gst_media_store_render (GstBaseSink * sink, GstBuffer * buffer)
{
  guint i, retval;
  sqlite3 *db;

  retval = sqlite3_open("/home/hms/vinay/media.db",&db);
  if( retval ){
	#ifdef DEBUG
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	#endif
        sqlite3_close(db);
        exit(1);
  }
  
  gchar *create_table = "CREATE TABLE IF NOT EXISTS songs (sname TEXT PRIMARY KEY)";
  retval = sqlite3_exec(db,create_table,0,0,0); 

  gchar **value = g_strsplit(GST_BUFFER_DATA (buffer), "\n", -1);  
  
  for(i = 0; value[i] != NULL; i++) {
    gchar *insert = g_strjoin("", "INSERT INTO songs VALUES(\'", value[i], "\')", NULL);
    retval = sqlite3_exec(db,insert,0,0,0);
    #ifdef DEBUG
    g_printf ("%s\n", value[i]);
    #endif
  }  

  #ifdef DEBUG
  g_printf ("Database render done\n");
  #endif
  g_strfreev(value); 

  return GST_FLOW_OK;
}

/* chain function
 * this function does the actual processing
 */
/*static GstFlowReturn
gst_media_store_chain (GstPad * pad, GstBuffer * buf)
{
  GstMediaStore *mediastore;

  mediastore = GST_MEDIASTORE (GST_OBJECT_PARENT (pad));

  if (mediastore->silent == FALSE)
    #ifdef DEBUG
    g_print ("I'm plugged, therefore I'm in.\n");
    #endif

  // just push out the incoming buffer without touching it
  return gst_pad_push (mediastore->srcpad, buf);
}*/


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
mediastore_init (GstPlugin * mediastore)
{
  /* debug category for fltering log messages
   *
   * exchange the string 'Template mediastore' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_media_store_debug, "mediastore",
      0, "Template mediastore");

  return gst_element_register (mediastore, "mediastore", GST_RANK_NONE,
      GST_TYPE_MEDIASTORE);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirstmediastore"
#endif

/* gstreamer looks for this structure to register mediastores
 *
 * exchange the string 'Template mediastore' with your mediastore description
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "mediastore",
    "Template mediastore",
    mediastore_init,
    VERSION,
    "LGPL",
    "GStreamer",
    "http://gstreamer.net/"
)
