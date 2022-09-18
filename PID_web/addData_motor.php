<?php
 // dang nhap vao database 
    $server = "localhost";
    $user = "tien"; 
    $pass = "368842";
    $dbname = "PID_controller";
    
    $conn = mysqli_connect($server,$user,$pass,$dbname);
 
 // Check connection
    if($conn === false){
        die("ERROR: Could not connect. " . mysqli_connect_error());
    }
    $Vel=(float)$_POST["vel"];
    $Pwm=(int)$_POST["pwm"];
    $Setpoint = (float)$_POST["sp"];
    $Kp = (float)$_POST["pv"];
    $Ki = (float)$_POST["iv"];
    $Kd = (float)$_POST["dv"];
    $sql = "insert into data(Vel,Pwm,Setpoint,Kp,Ki,Kd) values ($Vel,$Pwm,$Setpoint,$Kp,$Ki,$Kd);";

    
    mysqli_query($conn,$sql);
    mysqli_close($conn);
    echo($sql);
?>