public class reversenumber {


    public static void main(String[] args) {
       long given_number = 372392654143008L; 

        String reverse_number = reverseNumber(given_number);




    }


    public static String reverseNumber(long given_number) {

        //int number = 987654; 
        long c = 9876543210L;
        final StringBuilder sb = new StringBuilder();
        while (given_number  > 0){
            sb.append(given_number  % 10);
            given_number  /= 10;
        }
        System.out.println(sb.toString());

        return sb.toString();

         
    }  

}
    

