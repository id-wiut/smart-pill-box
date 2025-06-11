    function updateTime() {
      const now = new Date();
      document.getElementById("currentTime").textContent =
        "Current Time: " + now.toLocaleTimeString();
    }
    setInterval(updateTime, 1000);
    updateTime();

    function updateCountdown() {
      const now = new Date();
      const target = new Date();
      target.setHours(7, 0, 0, 0);
      if (now > target) {
        target.setDate(target.getDate() + 1); // next day
      }
      const diff = target - now;
      const hrs = Math.floor(diff / (1000 * 60 * 60));
      const mins = Math.floor((diff / (1000 * 60)) % 60);
      const secs = Math.floor((diff / 1000) % 60);

      document.getElementById("countdown").textContent =
        `Countdown until next watering: ${hrs}h ${mins}m ${secs}s`;
    }
    setInterval(updateCountdown, 1000);
    updateCountdown();

    function waterPlant() {
      alert("💧 Watering activated!");
    }

    const centerText = {
      id: 'centerText',
      afterDraw(chart) {
        const { ctx, chartArea: { width, height } } = chart;
        const value = chart.data.datasets[0].data[0];
        ctx.save();
        ctx.font = 'bold 16px Arial';
        ctx.fillStyle = '#333';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillText(`${value}%`, width / 2, height / 2);
        ctx.restore();
      }
    };

    const ctx = document.getElementById('humidityGauge').getContext('2d');
    const gauge = new Chart(ctx, {
      type: 'doughnut',
      data: {
        datasets: [{
          data: [65, 100 - 65],
          backgroundColor: [
            function(ctx) {
              const value = ctx.chart.data.datasets[0].data[0];
              if (value < 30) return '#e53935';    // red - dry
              if (value < 60) return '#fbc02d';    // yellow - moderate
              return '#43a047';                    // green - good
            },
            '#e0e0e0'
          ],
          borderWidth: 0,
          rotation: -90,
          circumference: 180,
          cutout: '70%'
        }]
      },
      options: {
        responsive: true,
        plugins: {
          legend: { display: false },
          tooltip: { enabled: false }
        }
      }
    });

    function updateHumidity(value) {
      value = Math.max(0, Math.min(100, value)); // Clamp between 0-100
      gauge.data.datasets[0].data = [value, 100 - value];
      gauge.update();
      document.getElementById("humidityValue").innerText = `Humidity: ${value} RH`;
    }

    setInterval(() => {
      const simulated = Math.floor(Math.random() * 100);
      updateHumidity(simulated);
    }, 3000);

    const ws = new WebSocket("ws://localhost:1880/ws/plant"); // adjust IP/port if needed

    ws.onmessage = (event) => {
      const data = JSON.parse(event.data);
      if (data.humidity !== undefined) {
        const humidity = Math.max(0, Math.min(100, data.humidity));
        humidityChart.data.datasets[0].data = [humidity, 100 - humidity];
        humidityChart.update();
      }
    };
