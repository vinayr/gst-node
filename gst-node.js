var express = require('express');
var net = require('net');
var child = require('child_process');
var fs = require('fs');

  // GStreamer command
  var cmd = 'gst-launch';
 
  // Create HTTP server
  var app = express.createServer();  

  require('jade');
  app.set('view engine', 'jade');
  app.set('view options', {layout: false});

  app.use(express.logger());
  app.use(express.methodOverride());
  app.use(express.bodyParser());
  app.use(app.router);
  app.use(express.static(__dirname + '/../../../../')); //root directory access.....change later

  app.get('/', function(req, res){
    res.render('index');
  });

/* SCAN module */
  app.get('/scan', function(req, res){

    //res.writeHead(200, {'Content-Type': 'text/html'});
    //res.header('Content-Type','text/html');

    var scanCmd = [ '-q', '--gst-plugin-path=/usr/local/lib/gstreamer-0.10', 
                    'mediascan', 'src=usb', 'num_buffers=1',
	            '!', 'mediastore'];
    var options = null;

    var pScan = child.spawn(cmd, scanCmd, options);

    pScan.stdout.on('data', function (data) {
      if (/^execvp\(\)/.test(data)) {
        console.log('failed to start child process\n');
      }
      else {    
        res.write(data);
        console.log('scan success');
      }
    });

    pScan.on('exit', function (code, signal) {
      if(code) {
        console.log('child process exited with code '+code);
	res.write('EXITCODE='+code);
      }
      else if(signal != null) {
        console.log('child process exited with signal '+signal);
	res.write('SIGNAL='+signal);
      }
      res.end();
    });

    res.connection.on('close', function() {
      console.log('Connection closed\n');
      pScan.kill();
    });
  });


/* STREAM module */
  app.get('/stream', function(req, res) {  

    var date = new Date();

    res.writeHead(200, {
      'Date':date.toUTCString(),
      'Connection':'close',
      'Cache-Control':'private',
      'Content-Type':'video/webm',
      //'Content-Type':'text/plain',
      //'Content-Type':'audio/mp3',
      'Server':'CustomStreamer/0.0.1',
    });


    //create tcp server
    var server = net.createServer(function (socket) {
      socket.on('data', function (data) {
        res.write(data); // data received from tcp will be sent as http response       
	//res.end();
      });
      socket.on('close', function(had_error) {
        res.end();
      });
    });

    /*var gst = child.spawn(cmd, args, options);

    gst.stdout.on('data', function(data) {
      res.write(data);
      console.log('stdout: ' + data);
    });

    gst.stderr.on('data', function(data) {
      //console.log(data.toString());
      console.log('On spawn error: ' + data);
    });

    gst.on('exit', function(code) {
      if (code != 0) {
        console.error('Process exited with code ' + code);
      }
      res.end();
    });

    res.connection.on('close', function() {
      console.log('Connection closed\n');
      gst.kill();
    });*/
  
  server.listen(function() {
   
    var playCmd = [ 'filesrc', 'location=/home/test/media/sample4.webm', 
	              //'!', 'mad',
	            '!', 'tcpclientsink', 'host=localhost', 'port='+server.address().port];
    var options = null;
                 // {env: {LD_LIBRARY_PATH: '/usr/local/lib',
                 // PATH: '/usr/local/bin:/usr/bin:/bin'}};

    var pPlay = child.spawn(cmd, playCmd, options);

    pPlay.stderr.on('data', function (data) {
      if (/^execvp\(\)/.test(data)) {
        console.log('failed to start child process\n');
      }
      else
        console.log('stderr: ' + data); //data.toString()
    });

    pPlay.on('exit', function (code) {
      if (code != null) {
        console.log('child process exited with code ' + code);
        res.end();
      }
    });

    res.connection.on('close', function() {
      console.log('Connection closed\n');
      pPlay.kill();
    });

  }); //end server listen


}); // end app get /


app.listen(8000);
console.log('Server running at http://127.0.0.1:8000/');

process.on('uncaughtException', function(err) {
  console.debug(err);
});
