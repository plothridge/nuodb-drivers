<?php 
// Test prepared statement with Integer parameter for NuoDB Timestamp column.
try {  
  $db = new PDO("nuodb:database=test@localhost;schema=Hockey", "dba", "goalie") or die;
  $db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
  $date = '1962-12-28';
  $sql = "select * from test1 where d = :date";
  $stmt = $db->prepare($sql);
  $stmt->bindParam(':date', $date, PDO::PARAM_STR);
  $stmt->execute();
  $result = $stmt->fetchAll();
  foreach ($result as $row) {
     print_r ($row);
  }
  $db = NULL;
} catch(PDOException $e) {  
  echo $e->getMessage();  
}
$db = NULL;  
echo "done\n";
?>
