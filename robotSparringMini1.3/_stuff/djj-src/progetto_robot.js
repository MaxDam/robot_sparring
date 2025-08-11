
document.addEventListener('DOMContentLoaded', function () {
  const stopButton = document.getElementById('Stop');
  const ganci = document.getElementById('cCB1');
  const diretti = document.getElementById('cCB2');

  stopButton.addEventListener('click', function () {
    ganci.checked = false;
    diretti.checked = false;
  });
});

