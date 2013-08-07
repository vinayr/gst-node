//compile: gcc -Wall playmedia.c -o playmedia $(pkg-config --cflags --libs gstreamer-0.10 sqlite3)
//run: ./playmedia

#include <gst/gst.h>
#include <glib.h>
#include <sqlite3.h>

static gboolean
bus_call (GstBus     *bus,
          GstMessage *msg,
          gpointer    data)
{
  GMainLoop *loop = (GMainLoop *) data;

  switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;

    case GST_MESSAGE_ERROR: {
      gchar  *debug;
      GError *error;

      gst_message_parse_error (msg, &error, &debug);
      g_free (debug);

      g_printerr ("Error: %s\n", error->message);
      g_error_free (error);

      g_main_loop_quit (loop);
      break;
    }
    default:
      break;
  }

  return TRUE;
}


static void
on_pad_added (GstElement *element,
              GstPad     *pad,
              gpointer    data)
{
  GstPad *sinkpad;
  GstElement *decoder = (GstElement *) data;

  /* We can now link this pad with the vorbis-decoder sink pad */
  g_print ("Dynamic pad created, linking demuxer/decoder\n");

  sinkpad = gst_element_get_static_pad (decoder, "sink");

  gst_pad_link (pad, sinkpad);

  gst_object_unref (sinkpad);
}



int
main (int   argc,
      char *argv[])
{
  GMainLoop *loop;

  GstElement *pipeline, *source, *demuxer, *decoder, *conv, *sink;
  GstBus *bus;

/***************SQLite operations*******************/
  int retval,i;
  gchar *val;
  sqlite3 *db;
  sqlite3_stmt *stmt;

  retval = sqlite3_open("/home/hms/vinay/media.db",&db);
  if(retval != SQLITE_OK) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    exit(1);
  }

  gchar *query = "SELECT min(sname) from songs";
  retval = sqlite3_prepare_v2(db,query,-1,&stmt,0);
  if(retval != SQLITE_OK) {
    g_print("Selecting data from DB Failed\n");
    return -1;
  }

  int cols = sqlite3_column_count(stmt);

  while(1) {
    retval = sqlite3_step(stmt);
    if(retval == SQLITE_ROW) {
      for(i=0 ; i<cols ; i++) {
        val = g_strdup_printf("%s",sqlite3_column_text(stmt,i));
        //g_print("%s = %s\t",sqlite3_column_name(stmt,i),val);
      }
      //g_print("\n");
    }
    else if(retval == SQLITE_DONE) {
      g_print("All rows fetched\n");
      break;
    }
    else {
      g_print("Some error encountered\n");
      return -1;
    }
  } 

/***************GStreamer operations*******************/

  /* Initialisation */
  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);

  /* Check input arguments */
  /*if (argc != 2) {
    g_printerr ("Usage: %s <Ogg/Vorbis filename>\n", argv[0]);
    return -1;
  }*/


  /* Create gstreamer elements */
  pipeline = gst_pipeline_new ("audio-player");
  source   = gst_element_factory_make ("filesrc",       "file-source");
  //demuxer  = gst_element_factory_make ("oggdemux",      "ogg-demuxer");
  //decoder  = gst_element_factory_make ("vorbisdec",     "vorbis-decoder");
  decoder  = gst_element_factory_make ("mad",     "decoder");
  //conv     = gst_element_factory_make ("audioconvert",  "converter");
  sink     = gst_element_factory_make ("autoaudiosink", "audio-output");

  if (!pipeline || !source || !decoder || !sink) {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }

  /* Set up the pipeline */


  /* we set the input filename to the source element */
  g_object_set (G_OBJECT (source), "location", val, NULL);

  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

  /* we add all elements into the pipeline */
  /* file-source | ogg-demuxer | vorbis-decoder | converter | alsa-output */
  gst_bin_add_many (GST_BIN (pipeline),
                    //source, demuxer, decoder, conv, sink, NULL);
                    source, decoder, sink, NULL);

  /* we link the elements together */
  /* file-source -> ogg-demuxer ~> vorbis-decoder -> converter -> alsa-output */
  //gst_element_link (source, demuxer);
  //gst_element_link_many (decoder, conv, sink, NULL);
  gst_element_link_many (source, decoder, sink, NULL);
  //g_signal_connect (demuxer, "pad-added", G_CALLBACK (on_pad_added), decoder);

  /* note that the demuxer will be linked to the decoder dynamically.
     The reason is that Ogg may contain various streams (for example
     audio and video). The source pad(s) will be created at run time,
     by the demuxer when it detects the amount and nature of streams.
     Therefore we connect a callback function which will be executed
     when the "pad-added" is emitted.*/


  /* Set the pipeline to "playing" state*/
  g_print ("Now playing: %s\n", argv[1]);
  gst_element_set_state (pipeline, GST_STATE_PLAYING);


  /* Iterate */
  g_print ("Running...\n");
  g_main_loop_run (loop);


  /* Out of the main loop, clean up nicely */
  g_print ("Returned, stopping playback\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);

  g_print ("Deleting pipeline\n");
  gst_object_unref (GST_OBJECT (pipeline));

  sqlite3_close(db);

  return 0;
}
