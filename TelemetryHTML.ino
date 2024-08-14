<!DOCTYPE HTML>
<html>
<head>
  <meta charset="utf-8">
  <title>Telemetry Data</title>
  <script src='https://cdn.plot.ly/plotly-latest.min.js'></script>
  <style>
    body {
      font-family: Georgia, serif;
      display: flex;
      flex-direction: column;
      align-items: center;
      background: linear-gradient(135deg, #121212, #2c3e50);
      margin: 0;
      padding: 20px;
      color: #FFFFFF;
      
    }
    .container {
      display: grid;
      align-items: center;
      grid-template-columns: repeat(3, 1fr);
      grid-gap: 20px;
      width: 90%;
      
      
    }
    .chart {
      width: 300px;
      height: 300px;
      background-color: #1E1E1E;
      border: solid 2px #FFD700;
      align-items: flex;
      padding:all;
      border-radius: 5px ;
      
      
    }
    h1, h2 {
      font-family: Georgia, serif;
      text-align: center;
      width: 100%;
      background: -webkit-linear-gradient(#FFD700, #FF5733);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
    }
    
  </style>
</head>
<body>
  <h1>Telemetry Data</h1>
  <div class='container'>
    <div class='chart' id='gyro'></div>
    <div class='chart' id='acc'></div>
    <div class='chart' id='altitude'></div>
    <div class='chart' id='pressure'></div>
    <div class='chart' id='temp'></div>
    <div class='chart' id='thrust'></div>
    <div class='chart'><h2>Avionic Status</h2><div id='status'></div></div>
  </div>
  <script>
    function updateData() {
      fetch('/data')
        .then(response => response.json())
        .then(data => {
          Plotly.update('gyro', { y: [[data.gyroX], [data.gyroY], [data.gyroZ]] });
          Plotly.update('acc', { y: [[data.accX], [data.accY], [data.accZ]] });
          Plotly.update('altitude', { y: [[data.altitude]] });
          Plotly.update('pressure', { y: [[data.pressure]] });
          Plotly.update('temp', { y: [[data.temp]] });
          Plotly.update('thrust', { y: [[data.thrust]] });
          document.getElementById('status').innerHTML = `
            IMU Connection: TRUE<br>
            Launch Detection: FALSE<br>
            Apogee Detection: FALSE<br>
            CNT Terminal: FALSE<br>
            Pyro Ejection: FALSE`;
        });
    }
    setInterval(updateData, 1000);
    Plotly.newPlot('gyro', [
      { y: [0], type: 'line', name: 'GyroX', line: {color: '#FFD700'} },
      { y: [0], type: 'line', name: 'GyroY', line: {color: '#FF5733'} },
      { y: [0], type: 'line', name: 'GyroZ', line: {color: '#C70039'} }
    ], { title: 'Gyroscopes', plot_bgcolor: "#1E1E1E", paper_bgcolor: "#1E1E1E", font: {color: '#FFFFFF'} });
    Plotly.newPlot('acc', [
      { y: [0], type: 'line', name: 'AccX', line: {color: '#FFD700'} },
      { y: [0], type: 'line', name: 'AccY', line: {color: '#FF5733'} },
      { y: [0], type: 'line', name: 'AccZ', line: {color: '#C70039'} }
    ], { title: 'Accelerometers', plot_bgcolor: "#1E1E1E", paper_bgcolor: "#1E1E1E", font: {color: '#FFFFFF'} });
    Plotly.newPlot('altitude', [{ y: [0], type: 'line', name: 'Altitude', line: {color: '#FFD700'} }], { title: 'Altitude', plot_bgcolor: "#1E1E1E", paper_bgcolor: "#1E1E1E", font: {color: '#FFFFFF'} });
    Plotly.newPlot('pressure', [{ y: [0], type: 'line', name: 'Pressure', line: {color: '#FFD700'} }], { title: 'Pressure', plot_bgcolor: "#1E1E1E", paper_bgcolor: "#1E1E1E", font: {color: '#FFFFFF'} });
    Plotly.newPlot('temp', [{ y: [0], type: 'line', name: 'Temperature', line: {color: '#FFD700'} }], { title: 'System Temperature', plot_bgcolor: "#1E1E1E", paper_bgcolor: "#1E1E1E", font: {color: '#FFFFFF'} });
    Plotly.newPlot('thrust', [{ y: [0], type: 'line', name: 'Thrust', line: {color: '#FFD700'} }], { title: 'Thrust', plot_bgcolor: "#1E1E1E", paper_bgcolor: "#1E1E1E", font: {color: '#FFFFFF'} });
  </script>
</body>
</html>
