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
 * SECTION:element-mediascan
 *
 * FIXME:Describe mediascan here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! mediascan ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

/*
TO COMPILE 
from gst-plugin directory
./autogen.sh
make
sudo make install
gst-launch -q --gst-plugin-path=/usr/local/lib/gstreamer-0.10 mediascan src=usb num_buffers=1 ! mediastore
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <string.h>
#include <stdlib.h>

#include "gstmediascan.h"

GST_DEBUG_CATEGORY_STATIC (gst_media_scan_debug);
#define GST_CAT_DEFAULT gst_media_scan_debug

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
  PROP_SRC
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

/*GST_BOILERPLATE (GstMediaScan, gst_media_scan, GstElement,
    GST_TYPE_ELEMENT);*/
GST_BOILERPLATE (GstMediaScan, gst_media_scan, GstPushSrc, 
    GST_TYPE_PUSH_SRC);

static void gst_media_scan_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_media_scan_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_media_scan_set_caps (GstPad * pad, GstCaps * caps);
//static GstFlowReturn gst_media_scan_chain (GstPad * pad, GstBuffer * buf);
static GstFlowReturn gst_media_scan_create (GstPushSrc * psrc, GstBuffer ** outbuf);

/*static GstStateChangeReturn
gst_media_scan_change_state (GstElement *element, GstStateChange transition);*/

/* GObject vmethod implementations */

static void
gst_media_scan_base_init (gpointer gclass)
{
  #ifdef DEBUG
  g_printf ("[%s]\n",__func__);
  #endif
  GstElementClass *element_class = GST_ELEMENT_CLASS (gclass);

  gst_element_class_set_details_simple(element_class,
    "Media Scan",
    "Source/Device",
    "Scan for media files",
    "Vinay <<user@hostname.org>>");

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_factory));
}

/* initialize the mediascan's class */
static void
gst_media_scan_class_init (GstMediaScanClass * klass)
{
  #ifdef DEBUG
  g_printf ("[%s]\n",__func__);
  #endif
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstPushSrcClass *gstpush_src_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstpush_src_class = GST_PUSH_SRC_CLASS (klass);

  gobject_class->set_property = gst_media_scan_set_property;
  gobject_class->get_property = gst_media_scan_get_property;

  //gstelement_class->change_state = gst_media_scan_change_state;

  g_object_class_install_property (gobject_class, 
      PROP_SRC,
      g_param_spec_string ("src",
          "src", "Source(usb/dlna)", NULL,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gstpush_src_class->create = GST_DEBUG_FUNCPTR (gst_media_scan_create);
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_media_scan_init (GstMediaScan * mediascan,
    GstMediaScanClass * gclass)
{
  #ifdef DEBUG
  g_printf ("[%s]\n",__func__);
  #endif
  mediascan->srctype = NULL;

  //mediascan->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
  //gst_pad_set_getcaps_function (mediascan->srcpad, GST_DEBUG_FUNCPTR(gst_pad_proxy_getcaps));
 // gst_element_add_pad (GST_ELEMENT (mediascan), mediascan->srcpad);

  mediascan->silent = FALSE;
}

/*static GstStateChangeReturn
gst_media_scan_change_state (GstElement *element, GstStateChange transition)
{
  #ifdef DEBUG
  g_printf ("[%s]\n",__func__);
  #endif
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
  GstMediaScan *mediascan = GST_MEDIASCAN (element);

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
      //if (!gst_media_scan_allocate_memory (mediascan))
        //return GST_STATE_CHANGE_FAILURE;
      break;
    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
  if (ret == GST_STATE_CHANGE_FAILURE)
    return ret;

  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_NULL:
      //gst_media_scan_free_memory (mediascan);
      break;
    default:
      break;
  }

  return ret;
}*/

static void
gst_media_scan_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  #ifdef DEBUG
  g_printf ("[%s]\n",__func__);
  #endif
  GstMediaScan *mediascan = GST_MEDIASCAN (object);

  switch (prop_id) {
    case PROP_SRC:
      if (mediascan->srctype) {
        g_free (mediascan->srctype);
        mediascan->srctype = NULL;
      }
      mediascan->srctype = g_value_get_string (value);
      #ifdef DEBUG
      g_printf ("Source - %s\n", mediascan->srctype);
      #endif
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_media_scan_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  #ifdef DEBUG
  g_printf ("[%s]\n",__func__);
  #endif
  GstMediaScan *mediascan = GST_MEDIASCAN (object);

  switch (prop_id) {
    case PROP_SRC:
      g_value_set_string (value, mediascan->srctype);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstElement vmethod implementations */

/* this function handles the link with other elements */
static gboolean
gst_media_scan_set_caps (GstPad * pad, GstCaps * caps)
{
  #ifdef DEBUG
  g_printf ("[%s]\n",__func__);
  #endif
  //GstPad *otherpad;
  GstMediaScan *mediascan;
  mediascan = GST_MEDIASCAN (gst_pad_get_parent (pad));
  //otherpad = (pad == mediascan->srcpad) ? mediascan->sinkpad : mediascan->srcpad;
  gst_object_unref (mediascan);

  //return gst_pad_set_caps (otherpad, caps);
  return gst_pad_set_caps (mediascan->srcpad, caps);
}

/* chain function
 * this function does the actual processing
 */
/*static GstFlowReturn
gst_media_scan_chain (GstPad * pad, GstBuffer * buf)
{
  #ifdef DEBUG
  g_printf ("[%s]\n",__func__);
  #endif
  GstMediaScan *mediascan;
  GstBuffer *strbuf;
  guint blocksize = 40;
  char string[] = "This is media scanner plugin yo";

  mediascan = GST_MEDIASCAN (GST_OBJECT_PARENT (pad));

  strbuf = gst_buffer_try_new_and_alloc (blocksize);
  if (G_UNLIKELY (buf == NULL)) {
    GST_ERROR_OBJECT (pad, "Failed to allocate %u bytes", blocksize);
    return GST_FLOW_ERROR;
  }
  
  memcpy(strbuf, string, strlen(string)+1);

  if (mediascan->src == FALSE)
    #ifdef DEBUG
    g_print ("I'm plugged, therefore I'm in.\n");
    #endif

  // just push out the incoming buffer without touching it
  return gst_pad_push (mediascan->srcpad, strbuf);
}*/

static GstFlowReturn
gst_media_scan_create (GstPushSrc * psrc, GstBuffer ** outbuf)
{
  #ifdef DEBUG
  g_printf ("[%s]\n",__func__);
  #endif
  FILE *fpipe;
  guint size = 2048; //check this later
  guint result;

  GstMediaScan *mediascan;
  GstBuffer *buf, *tmpbuf;  

  mediascan = GST_MEDIASCAN (psrc);

  tmpbuf = gst_buffer_try_new_and_alloc (size);
  if (G_UNLIKELY (tmpbuf == NULL)) {
    GST_ERROR_OBJECT (mediascan, "Failed to allocate %u bytes", size);
    return GST_FLOW_ERROR;
  }

  //gchar *dir = "/media/USB\\ Drive";
  gchar *dir = "/home/test/media";
  gchar *mediatype = "mp3|ogg|webm|flac|wav|wma|aac|ra|midi|aiff|m4a|avi|webm|wmv|mp4|mpeg|mpg|mov|flv|rm";

  gchar *command = g_strjoin("", "find ", dir, " -regextype awk -iregex '.*\.(", mediatype,")$'", NULL);
  #ifdef DEBUG
  g_printf("command: %s\n", command);
  #endif
  if ((fpipe = (FILE*)popen(command, "r")) <= 0) {
    perror("popen() failed.");
    return GST_FLOW_ERROR;
  }

  result = fread(GST_BUFFER_DATA (tmpbuf), 1, size, fpipe);
  #ifdef DEBUG
  g_printf("result-%d, size-%d\n", result, size);
  #endif
  --result; //truncate '/n' at the end
  buf = gst_buffer_try_new_and_alloc (result);

  memcpy(GST_BUFFER_DATA (buf), GST_BUFFER_DATA (tmpbuf), result);
  g_printf ("%s", GST_BUFFER_DATA (buf)); //send to nodejs

  /*gchar **value = g_strsplit(GST_BUFFER_DATA (buf), "\n", -1);  
  int i;
  for(i = 0; value[i] != NULL; i++) {
    g_printf ("%s\n", value[i]);
  }
  g_strfreev(value); 
*/

  *outbuf = buf; //copy output buffer to pipeline

  pclose(fpipe);

  return GST_FLOW_OK;
}

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
mediascan_init (GstPlugin * mediascan)
{
  #ifdef DEBUG
  g_printf ("[%s]\n",__func__);
  #endif
  /* debug category for fltering log messages
   *
   * exchange the string 'Template mediascan' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_media_scan_debug, "mediascan",
      0, "Template mediascan");

  return gst_element_register (mediascan, "mediascan", GST_RANK_NONE,
      GST_TYPE_MEDIASCAN);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirstmediascan"
#endif

/* gstreamer looks for this structure to register mediascans
 *
 * exchange the string 'Template mediascan' with your mediascan description
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "mediascan",
    "Template mediascan",
    mediascan_init,
    VERSION,
    "LGPL",
    "GStreamer",
    "http://gstreamer.net/"
)
