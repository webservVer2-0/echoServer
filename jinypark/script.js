var canvas = document.getElementById("myCanvas");
var context = canvas.getContext("2d");

context.fillStyle = "green";
context.beginPath();
context.arc(75, 75, 50, 0, Math.PI*2);
context.fill();
