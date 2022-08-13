
import java.util.Scanner;
import java.net.HttpURLConnection;
import java.net.URL;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import com.google.gson.JsonParser;
import com.google.gson.JsonObject;


//Rreferences - https://www.codejava.net/java-se/networking/how-to-use-java-urlconnection-and-httpurlconnection
// https://github.com/eugenp/tutorials/blob/master/core-java-modules/core-java-networking-2/src/main/java/com/baeldung/urlconnection/PostJSONWithHttpURLConnection.java
//api.openweathermap.org/data/2.5/weather?q={city name},{state code},{country code}&appid={API key}

public class weatherAPI {




    public static void main(String[] args) throws IOException

    {
        // accept input for city or zip
        // enter city for the weather
        //or enter the zip for the weather

        // call the api

        // url - http://api.openweathermap.org/data/2.5/weather?q=Austin&APPID=26aa1d90a24c98fad4beaac70ddbf274

        

        Scanner reader = new Scanner(System.in);  // Reading from System.in
        System.out.println("Enter a city for the weather : ");
        String city = reader.next(); // Scans the next token of the input as an int.
        System.out.println("Enter two character state code of the City : ");
        String state_code = reader.next();
        System.out.println("Enter two character Country code of the State such as 'US','FR','IN' etc. : ");
        String country_code = reader.next();
        //once finished
        reader.close();

        System.out.println("City:" + city);
        System.out.println("State:" + state_code);
        System.out.println("Country:" + country_code);
        String api_key = "151b0fb5cc11b727d3d3cd385695c37b";

        String endpoint = "http://api.openweathermap.org/data/2.5/weather";
        //?q={city name},{state code},{country code}&appid={API key}&units=imperial
        // return temp in fahrenheit; default is kelvin ; for celcius the value would be metric
        String params = "?q=" + city + "," + state_code +"," + country_code + "&appid=" + api_key + "&units=imperial";

        URL url = new URL(endpoint + params);


        //URL url = new URL ("http://api.openweathermap.org/data/2.5/weather?q=75013&appid=151b0fb5cc11b727d3d3cd385695c37b");

        // Map<String, String> parameters = new HashMap<>();
        // parameters.put("q", city);
        // parameters.put("appid", "151b0fb5cc11b727d3d3cd385695c37b");

        //Ravis API-KEY - 151b0fb5cc11b727d3d3cd385695c37b


		HttpURLConnection con = (HttpURLConnection)url.openConnection();
		
		con.setRequestMethod("GET");
		
		con.setRequestProperty("Content-Type", "application/json; utf-8");
		con.setRequestProperty("Accept", "application/json");
        con.setDoOutput(true);

        // read connection input stream
   	
		


        StringBuilder response = new StringBuilder();

        try(BufferedReader br = new BufferedReader(new InputStreamReader(con.getInputStream(), "utf-8"))){
			//StringBuilder response = new StringBuilder();
			String responseLine = null;
			while ((responseLine = br.readLine()) != null) {
				response.append(responseLine.trim());
			}
            System.out.println("response :");
			System.out.println(response.toString());
            System.out.println("");
		}
		
        /**
		//JSON String need to be constructed for the specific resource. 
		//We may construct complex JSON using any third-party JSON libraries such as jackson or org.json
		String jsonInputString = "{\"name\": \"Upendra\", \"job\": \"Programmer\"}";
		
		try(OutputStream os = con.getOutputStream()){
			byte[] input = jsonInputString.getBytes("utf-8");
			os.write(input, 0, input.length);			
		}
        */

		int code = con.getResponseCode();
		System.out.println("http response code:" + code);
		
		double temparature = jsonParser(response.toString());

        System.out.println("temparature:" + temparature);
	}

    public static double jsonParser(String jsoninput) {

        JsonObject jsonObject = new JsonParser().parse(jsoninput).getAsJsonObject();

        
        return (jsonObject.get("main").getAsJsonObject().get("temp").getAsDouble()); 

        
    }


    
        
    



}
    

