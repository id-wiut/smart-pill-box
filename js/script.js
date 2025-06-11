ws.onmessage = function(event) {
      const data = JSON.parse(event.data);

      document.getElementById("lid-status").textContent = data.lid;
      document.getElementById("dosage-time").textContent = data.time === "yes" ? "Yes ✅" : "No ❌";
      document.getElementById("doses").textContent = data.doses;
      document.getElementById("last-time").textContent = data.last;
      
      const missedEl = document.getElementById("missed");
      missedEl.textContent = data.missed ? "Yes ❗" : "No ✅";
      missedEl.className = data.missed ? "fw-bold badge badge-red" : "fw-bold badge badge-green";

      document.getElementById("weight").textContent = data.weight + " g";
      document.getElementById("led").textContent = data.led === "green" ? "Green ✅" : "Red ⚠️";
      document.getElementById("buzzer").textContent = data.buzzer === "on" ? "ON 🔔" : "OFF 🔕";
    };